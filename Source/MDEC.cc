#include "Global.h"


/* PSX 15-bit color is BGR555: R in bits 0-4, G in 5-9, B in 10-14. */
#define    MAKERGB15(R, G, B) ((((B)>>3)<<10)|(((G)>>3)<<5)|((R)>>3)|bit15)
#define    ROUND(c) rtbl[(c) + 128 + 256]

#define RGB15CL(N) IMAGE[N] = MAKERGB15(ROUND(GETY + R), ROUND(GETY + G), ROUND(GETY + B));
#define RGB24CL(N) IMAGE[N+ 2] = ROUND(GETY + R); IMAGE[N+ 1] = ROUND(GETY + G); IMAGE[N+ 0] = ROUND(GETY + B);

#define    MULR(A) (((sw)0x0000059B * (A)) >> 10)
#define    MULG(A) (((sw)0xFFFFFEA1 * (A)) >> 10)
#define    MULB(A) (((sw)0x00000716 * (A)) >> 10)
#define    MULF(A) (((sw)0xFFFFFD25 * (A)) >> 10)

#define    RUNOF(a) ((a)>>10)
#define    VALOF(a) ((sw)(((a)<<22)>>22))


CstrMotionDecoder mdec;

static sw iq_y[64], iq_uv[64];

void CstrMotionDecoder::updateStatus() {
    status = STAT_FIFO_EMPTY;
    status |= ((cmd >> 25) & 0x0f) << 23; // depth/signed/bit15 mirror
    status |= (4 << STAT_BLOCK_SHIFT);    // idle block = Cr

    if (busy) {
        status |= STAT_BUSY;
        status &= ~STAT_FIFO_EMPTY;
        status |= STAT_FIFO_FULL;
    }

    if (paramWords) {
        status = (status & ~0xffff) | ((paramWords - 1) & 0xffff);
    }
    else {
        status |= 0xffff;
    }

    if (dinEnable && busy && paramWords) {
        status |= STAT_DIN_REQ;
    }

    if (doutEnable && outReady) {
        status |= STAT_DOUT_REQ;
        status &= ~STAT_FIFO_EMPTY;
    }
}

void CstrMotionDecoder::setParamWords(uw words) {
    paramWords = words;
}

void CstrMotionDecoder::reflectCmdBits(uw data) {
    // Bits 28-25 of the command are mirrored into status 26-23.
    cmd = (cmd & ~0x1e000000) | (data & 0x1e000000);
}

void CstrMotionDecoder::reset() {
    rl = (uh *)&mem.ram.ptr[0x100000];
    cmd = 0;
    dinEnable = doutEnable = false;
    busy = outReady = false;
    paramWords = 0;
    len = 0;
    bit15 = 0;
    status = STAT_RESET_VALUE;

    for (sw k=0; k<256; k++) {
        rtbl[k+0x000] = 0;
        rtbl[k+0x100] = k;
        rtbl[k+0x200] = 255;
    }
}

void CstrMotionDecoder::write(uw addr, uw data) {
    switch(addr & 0xf) {
        case 0: {
            cmd = data;
            reflectCmdBits(data);
            bit15 = (data & 0x02000000) ? 0x8000 : 0;

            const uw opcode = data & CMD_MASK;

            if (opcode == CMD_DECODE) {
                len = data & 0xffff;
                setParamWords(len);
                busy = true;
                outReady = false;
            }
            else if (opcode == CMD_QUANT) {
                // 64 bytes luminance, optional +64 bytes chrominance
                setParamWords((data & 1) ? 32 : 16);
                busy = true;
                outReady = false;
            }
            else if (opcode == CMD_SCALE) {
                // 64 signed halfwords = 32 words
                setParamWords(32);
                busy = true;
                outReady = false;
            }
            else {
                // NOP / invalid: reflect size bits without expecting DMA
                setParamWords(0);
                busy = false;
                outReady = false;
                updateStatus();
                status = (status & ~0xffff) | (data & 0xffff);
                return;
            }

            updateStatus();
            return;
        }

        case 4:
            if (data & CTRL_RESET) {
                reset();
                return;
            }

            dinEnable  = (data & CTRL_DIN_ENABLE)  != 0;
            doutEnable = (data & CTRL_DOUT_ENABLE) != 0;
            updateStatus();
            return;
    }

    printx("/// PSeudo MDEC write: 0x%08x <- 0x%08x", addr, data);
}

uw CstrMotionDecoder::read(uw addr) {
    switch(addr & 0xf) {
        case 0:
            return cmd;

        case 4:
            return status;
    }

    printx("/// PSeudo MDEC read: 0x%08x", addr);
    return 0;
}

void CstrMotionDecoder::MacroBlock(sw *block, sw kh, sw sh) {
    for (sw k=0; k<8; k++, (sh) ? block+=8 : block++) {
        if((block[kh*1]|
            block[kh*2]|
            block[kh*3]|
            block[kh*4]|
            block[kh*5]|
            block[kh*6]|
            block[kh*7]) == 0) {
            block[kh*0]=
            block[kh*1]=
            block[kh*2]=
            block[kh*3]=
            block[kh*4]=
            block[kh*5]=
            block[kh*6]=
            block[kh*7]=
            block[kh*0]>>sh;

            continue;
        }
        sw z10 = block[kh*0]+block[kh*4];
        sw z11 = block[kh*0]-block[kh*4];
        sw z13 = block[kh*2]+block[kh*6];
        sw z12 = block[kh*2]-block[kh*6]; z12 = ((z12*362)>>8)-z13;

        sw tmp0 = z10+z13;
        sw tmp3 = z10-z13;
        sw tmp1 = z11+z12;
        sw tmp2 = z11-z12;

        z13 = block[kh*3]+block[kh*5];
        z10 = block[kh*3]-block[kh*5];
        z11 = block[kh*1]+block[kh*7];
        z12 = block[kh*1]-block[kh*7]; sw z5 = (((z12-z10)*473)>>8);

        sw tmp7 = z11+z13;
        sw tmp6 = (((z10)*669)>>8)+z5 -tmp7;
        sw tmp5 = (((z11-z13)*362)>>8)-tmp6;
        sw tmp4 = (((z12)*277)>>8)-z5 +tmp5;

        block[kh*0] = (tmp0+tmp7)>>sh;
        block[kh*7] = (tmp0-tmp7)>>sh;
        block[kh*1] = (tmp1+tmp6)>>sh;
        block[kh*6] = (tmp1-tmp6)>>sh;
        block[kh*2] = (tmp2+tmp5)>>sh;
        block[kh*5] = (tmp2-tmp5)>>sh;
        block[kh*4] = (tmp3+tmp4)>>sh;
        block[kh*3] = (tmp3-tmp4)>>sh;
    }
}

void CstrMotionDecoder::idct(sw *block, sw k) {
    if (k == 0) {
        sw val = block[0]>>5;

        for (sw i=0; i<64; i++) {
            block[i] = val;
        }
        return;
    }
    MacroBlock(block, 8, 0);
    MacroBlock(block, 1, 5);
}

void CstrMotionDecoder::TabInit(sw *iqtab, ub *src) {
    for (sw i=0; i<64; i++) {
        iqtab[i] = src[i]*aanscales[zscan[i]]>>12;
    }
}

uh *CstrMotionDecoder::rl2blk(sw *blk, uh *mdec_rl) {
    sw k,q_scale,rl;
    sw *iqtab;

    memset(blk, 0, 6*64*4);
    iqtab = iq_uv;

    for (sw i=0; i<6; i++) {

        if (i>1) iqtab = iq_y;

        // Skip FE00h padding at the start of a block
        do {
            rl = *mdec_rl++;
        } while (rl == 0xfe00);

        q_scale = rl>>10;
        blk[0] = iqtab[0]*VALOF(rl);
        k = 0;

        for(;;) {
            rl = *mdec_rl++; if (rl==0xfe00) break;
            k += (rl>>10)+1;if (k >  63) break;
            blk[zscan[k]] = (iqtab[k] * q_scale * VALOF(rl)) >> 3;
        }
        idct(blk, k+1);

        blk+=64;
    }
    return mdec_rl;
}

void CstrMotionDecoder::Yuv15(sw *Block, uh *IMAGE) {
    sw GETY;
    sw CB,CR,R,G,B;

    sw *YYBLK = Block + 64 * 2;
    sw *CBBLK = Block;
    sw *CRBLK = Block + 64;

    for (sw Y=0; Y<16; Y+=2, CRBLK+=4, CBBLK+=4, YYBLK+=8, IMAGE+=24) {
        if (Y == 8) {
            YYBLK = YYBLK + 64;
        }

        for (sw X=0; X<4; X++, IMAGE+=2, CRBLK++, CBBLK++, YYBLK+=2) {
            CR = *CRBLK;
            CB = *CBBLK;

            R = MULR(CR);
            G = MULG(CB) + MULF(CR);
            B = MULB(CB);

            GETY = YYBLK[0]; RGB15CL(0x00);
            GETY = YYBLK[1]; RGB15CL(0x01);
            GETY = YYBLK[8]; RGB15CL(0x10);
            GETY = YYBLK[9]; RGB15CL(0x11);

            CR = *(CRBLK + 4);
            CB = *(CBBLK + 4);

            R = MULR(CR);
            G = MULG(CB) + MULF(CR);
            B = MULB(CB);

            GETY = YYBLK[64 + 0]; RGB15CL(0x08);
            GETY = YYBLK[64 + 1]; RGB15CL(0x09);
            GETY = YYBLK[64 + 8]; RGB15CL(0x18);
            GETY = YYBLK[64 + 9]; RGB15CL(0x19);
        }
    }
}

void CstrMotionDecoder::Yuv24(sw *Block, ub *IMAGE) {
    sw GETY;
    sw CB, CR, R, G, B;

    sw *YYBLK = Block + 64 * 2;
    sw *CBBLK = Block;
    sw *CRBLK = Block + 64;

    for (sw Y=0; Y<16; Y+=2, CRBLK+=4, CBBLK+=4, YYBLK+=8, IMAGE+=24*3) {
        if (Y == 8) {
            YYBLK = YYBLK + 64;
        }

        for (sw X=0; X<4; X++, IMAGE+=2*3, CRBLK++, CBBLK++, YYBLK+=2) {
            CR = *CRBLK;
            CB = *CBBLK;

            R = MULR(CR);
            G = MULG(CB) + MULF(CR);
            B = MULB(CB);

            GETY = YYBLK[0]; RGB24CL(0x00 * 3);
            GETY = YYBLK[1]; RGB24CL(0x01 * 3);
            GETY = YYBLK[8]; RGB24CL(0x10 * 3);
            GETY = YYBLK[9]; RGB24CL(0x11 * 3);

            CR = *(CRBLK + 4);
            CB = *(CBBLK + 4);

            R = MULR(CR);
            G = MULG(CB) + MULF(CR);
            B = MULB(CB);

            GETY = YYBLK[64 + 0]; RGB24CL(0x08 * 3);
            GETY = YYBLK[64 + 1]; RGB24CL(0x09 * 3);
            GETY = YYBLK[64 + 8]; RGB24CL(0x18 * 3);
            GETY = YYBLK[64 + 9]; RGB24CL(0x19 * 3);
        }
    }
}

void CstrMotionDecoder::executeDMA(CstrBus::castDMA *dma) {
    ub *p = &mem.ram.ptr[dma->madr&(mem.ram.size-1)];
    sw z = (dma->bcr>>16)*(dma->bcr&0xffff);

    switch (dma->chcr&0xfff) {
        case 0x200: // DMA1 out — decoded RGB to RAM
        {
            sw blocksize, blk[384];
            uh *im = (uh *)p;
            const bool depth15 = ((cmd >> 27) & 3) == 3;

            if (depth15) {
                blocksize = 256;
            }
            else {
                blocksize = 384;
            }

            for (; z>0; z-=blocksize/2, im+=blocksize) {
                rl = rl2blk(blk, rl);

                if (depth15) {
                    Yuv15(blk, im);
                }
                else {
                    Yuv24(blk, (ub *)im);
                }
            }

            outReady = false;
            busy = false;
            updateStatus();
            break;
        }

        case 0x201: // DMA0 in — quant / scale / bitstream
        {
            const uw opcode = cmd & CMD_MASK;
            const uw consumed = (uw)z;

            if (opcode == CMD_QUANT) {
                TabInit(iq_y, p);
                if (cmd & 1) {
                    TabInit(iq_uv, p + 64);
                }
            }
            else if (opcode == CMD_SCALE) {
                // Scale table accepted; fast IDCT uses the built-in AAN table.
            }
            else if (opcode == CMD_DECODE) {
                rl = (uh *)p;
                outReady = true;
            }

            if (paramWords > consumed) {
                paramWords -= consumed;
            }
            else {
                paramWords = 0;
                busy = false;
            }

            updateStatus();
            break;
        }
    }
}
