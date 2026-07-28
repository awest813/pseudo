#include "Global.h"


#undef RGB

// 32-bit accessor
#define oooo(base, index) \
    base[(index)]

// 16-bit accessor
#define __oo(base, index, offset) \
    base[(index << 1) + offset]

// 08-bit accessor
#define ___o(base, index, offset) \
    base[(index << 2) + offset]

// Cop2c
#define R11R12 oooo(cop2c.sw,  0)    /* Rotation matrix */
#define R11    __oo(cop2c.sh,  0, 0) /* Rotation matrix */
#define R12    __oo(cop2c.sh,  0, 1) /* Rotation matrix */
#define R13    __oo(cop2c.sh,  1, 0) /* Rotation matrix */
#define R21    __oo(cop2c.sh,  1, 1) /* Rotation matrix */
#define R22R23 oooo(cop2c.sw,  2)    /* Rotation matrix */
#define R22    __oo(cop2c.sh,  2, 0) /* Rotation matrix */
#define R23    __oo(cop2c.sh,  2, 1) /* Rotation matrix */
#define R31    __oo(cop2c.sh,  3, 0) /* Rotation matrix */
#define R32    __oo(cop2c.sh,  3, 1) /* Rotation matrix */
#define R33    __oo(cop2c.sh,  4, 0) /* Rotation matrix */
#define TRX    oooo(cop2c.sw,  5)    /* Translation vector (X) */
#define TRY    oooo(cop2c.sw,  6)    /* Translation vector (Y) */
#define TRZ    oooo(cop2c.sw,  7)    /* Translation vector (Z) */
#define L11    __oo(cop2c.sh,  8, 0) /* Lite source direction vector X 3 */
#define L12    __oo(cop2c.sh,  8, 1) /* Lite source direction vector X 3 */
#define L13    __oo(cop2c.sh,  9, 0) /* Lite source direction vector X 3 */
#define L21    __oo(cop2c.sh,  9, 1) /* Lite source direction vector X 3 */
#define L22    __oo(cop2c.sh, 10, 0) /* Lite source direction vector X 3 */
#define L23    __oo(cop2c.sh, 10, 1) /* Lite source direction vector X 3 */
#define L31    __oo(cop2c.sh, 11, 0) /* Lite source direction vector X 3 */
#define L32    __oo(cop2c.sh, 11, 1) /* Lite source direction vector X 3 */
#define L33    __oo(cop2c.sh, 12, 0) /* Lite source direction vector X 3 */
#define RBK    oooo(cop2c.sw, 13)    /* Peripheral color (R) */
#define GBK    oooo(cop2c.sw, 14)    /* Peripheral color (G) */
#define BBK    oooo(cop2c.sw, 15)    /* Peripheral color (B) */
#define LR1    __oo(cop2c.sh, 16, 0) /* Lite source color X 3 */
#define LR2    __oo(cop2c.sh, 16, 1) /* Lite source color X 3 */
#define LR3    __oo(cop2c.sh, 17, 0) /* Lite source color X 3 */
#define LG1    __oo(cop2c.sh, 17, 1) /* Lite source color X 3 */
#define LG2    __oo(cop2c.sh, 18, 0) /* Lite source color X 3 */
#define LG3    __oo(cop2c.sh, 18, 1) /* Lite source color X 3 */
#define LB1    __oo(cop2c.sh, 19, 0) /* Lite source color X 3 */
#define LB2    __oo(cop2c.sh, 19, 1) /* Lite source color X 3 */
#define LB3    __oo(cop2c.sh, 20, 0) /* Lite source color X 3 */
#define RFC    oooo(cop2c.sw, 21)    /* Far color (R) */
#define GFC    oooo(cop2c.sw, 22)    /* Far color (G) */
#define BFC    oooo(cop2c.sw, 23)    /* Far color (B) */
#define OFX    oooo(cop2c.sw, 24)    /* Screen offset (X) */
#define OFY    oooo(cop2c.sw, 25)    /* Screen offset (Y) */
#define H      __oo(cop2c.sh, 26, 0) /* Screen position */
#define DQA    __oo(cop2c.sh, 27, 0) /* Depth parameter A (coefficient) */
#define DQB    oooo(cop2c.sw, 28)    /* Depth parameter B (offset) */
#define ZSF3   __oo(cop2c.sh, 29, 0) /* Z scale factor */
#define ZSF4   __oo(cop2c.sh, 30, 0) /* Z scale factor */
#define FLAG   oooo(cop2c.uw, 31)    /* ! */

// Cop2d
#define VXY0   oooo(cop2d.uw,  0)    /* Vector #0 (X/Y) */
#define VX0    __oo(cop2d.sh,  0, 0) /* Vector #0 (X) */
#define VY0    __oo(cop2d.sh,  0, 1) /* Vector #0 (Y) */
#define VZ0    __oo(cop2d.sh,  1, 0) /* Vector #0 (Z) */
#define VXY1   oooo(cop2d.uw,  2)    /* Vector #1 (X/Y) */
#define VX1    __oo(cop2d.sh,  2, 0) /* Vector #1 (X) */
#define VY1    __oo(cop2d.sh,  2, 1) /* Vector #1 (Y) */
#define VZ1    __oo(cop2d.sh,  3, 0) /* Vector #1 (Z) */
#define VXY2   oooo(cop2d.uw,  4)    /* Vector #2 (X/Y) */
#define VX2    __oo(cop2d.sh,  4, 0) /* Vector #2 (X) */
#define VY2    __oo(cop2d.sh,  4, 1) /* Vector #2 (Y) */
#define VZ2    __oo(cop2d.sh,  5, 0) /* Vector #2 (Z) */
#define RGB    oooo(cop2d.uw,  6)    /* Color data +  instruction */
#define R      ___o(cop2d.ub,  6, 0) /* Color (R) */
#define G      ___o(cop2d.ub,  6, 1) /* Color (G) */
#define B      ___o(cop2d.ub,  6, 2) /* Color (B) */
#define CODE   ___o(cop2d.ub,  6, 3) /*  instruction */
#define OTZ    __oo(cop2d.uh,  7, 0) /* Z-component value */
#define IR0    __oo(cop2d.sh,  8, 0) /* Intermediate value #0 */
#define IR1    __oo(cop2d.sh,  9, 0) /* Intermediate value #1 */
#define IR2    __oo(cop2d.sh, 10, 0) /* Intermediate value #2 */
#define IR3    __oo(cop2d.sh, 11, 0) /* Intermediate value #3 */
#define SXY0   oooo(cop2d.uw, 12)    /* Calc. result record #0 (X/Y) */
#define SX0    __oo(cop2d.sh, 12, 0) /* Calc. result record #0 (X) */
#define SY0    __oo(cop2d.sh, 12, 1) /* Calc. result record #0 (Y) */
#define SXY1   oooo(cop2d.uw, 13)    /* Calc. result record #1 (X/Y) */
#define SX1    __oo(cop2d.sh, 13, 0) /* Calc. result record #1 (X) */
#define SY1    __oo(cop2d.sh, 13, 1) /* Calc. result record #1 (Y) */
#define SXY2   oooo(cop2d.uw, 14)    /* Calc. result record #2 (X/Y) */
#define SX2    __oo(cop2d.sh, 14, 0) /* Calc. result record #2 (X) */
#define SY2    __oo(cop2d.sh, 14, 1) /* Calc. result record #2 (Y) */
#define SXYP   oooo(cop2d.uw, 15)    /* Calc. result set (X/Y) */
#define SXP    __oo(cop2d.sh, 15, 0) /* Calc. result set (X) */
#define SYP    __oo(cop2d.sh, 15, 1) /* Calc. result set (Y) */
#define SZ0    __oo(cop2d.uh, 16, 0) /* Calc. result record #0 (Z) */
#define SZ1    __oo(cop2d.uh, 17, 0) /* Calc. result record #1 (Z) */
#define SZ2    __oo(cop2d.uh, 18, 0) /* Calc. result record #2 (Z) */
#define SZ3    __oo(cop2d.uh, 19, 0) /* Calc. result record #3 (Z) */
#define RGB0   oooo(cop2d.uw, 20)    /* Calc. result record #0 (R/G/B) */
#define R0     ___o(cop2d.ub, 20, 0) /* Calc. result record #0 (R) */
#define G0     ___o(cop2d.ub, 20, 1) /* Calc. result record #0 (G) */
#define B0     ___o(cop2d.ub, 20, 2) /* Calc. result record #0 (B) */
#define CODE0  ___o(cop2d.ub, 20, 3) /* Calc. result record #0 (?) */
#define RGB1   oooo(cop2d.uw, 21)    /* Calc. result record #1 (R/G/B) */
#define R1     ___o(cop2d.ub, 21, 0) /* Calc. result record #1 (R) */
#define G1     ___o(cop2d.ub, 21, 1) /* Calc. result record #1 (G) */
#define B1     ___o(cop2d.ub, 21, 2) /* Calc. result record #1 (B) */
#define CODE1  ___o(cop2d.ub, 21, 3) /* Calc. result record #1 (?) */
#define RGB2   oooo(cop2d.uw, 22)    /* Calc. result record #2 (R/G/B) */
#define R2     ___o(cop2d.ub, 22, 0) /* Calc. result record #2 (R) */
#define G2     ___o(cop2d.ub, 22, 1) /* Calc. result record #2 (G) */
#define B2     ___o(cop2d.ub, 22, 2) /* Calc. result record #2 (B) */
#define CODE2  ___o(cop2d.ub, 22, 3) /* Calc. result record #2 (?) */
#define RES1   oooo(cop2d.uw, 23)    /* Reserved (access prohibited) */
#define MAC0   oooo(cop2d.sw, 24)    /* Sum of products #0 */
#define MAC1   oooo(cop2d.sw, 25)    /* Sum of products #1 */
#define MAC2   oooo(cop2d.sw, 26)    /* Sum of products #2 */
#define MAC3   oooo(cop2d.sw, 27)    /* Sum of products #3 */
#define IRGB   oooo(cop2d.uw, 28)    /* Color data input */
#define ORGB   oooo(cop2d.uw, 29)    /* Color data output */
#define LZCS   oooo(cop2d.uw, 30)    /* Lead zero/one count source data */
#define LZCR   oooo(cop2d.uw, 31)    /* Lead zero/one count process result */

#define MX11(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 0), 0) : 0)
#define MX12(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 0), 1) : 0)
#define MX13(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 1), 0) : 0)
#define MX21(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 1), 1) : 0)
#define MX22(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 2), 0) : 0)
#define MX23(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 2), 1) : 0)
#define MX31(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 3), 0) : 0)
#define MX32(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 3), 1) : 0)
#define MX33(n) (n < 3 ? __oo(cop2c.sh, ((n << 3) + 4), 0) : 0)

#define CV1( n) (n < 3 ? oooo(cop2c.sw, ((n << 3) + 5)) : 0)
#define CV2( n) (n < 3 ? oooo(cop2c.sw, ((n << 3) + 6)) : 0)
#define CV3( n) (n < 3 ? oooo(cop2c.sw, ((n << 3) + 7)) : 0)

#define VX(  n) (n < 3 ? __oo(cop2d.sh, ((n << 1) + 0), 0) : __oo(cop2d.sh,  9, 0))
#define VY(  n) (n < 3 ? __oo(cop2d.sh, ((n << 1) + 0), 1) : __oo(cop2d.sh, 10, 0))
#define VZ(  n) (n < 3 ? __oo(cop2d.sh, ((n << 1) + 1), 0) : __oo(cop2d.sh, 11, 0))

#define SX(n) __oo(cop2d.sh, (n + 12), 0)
#define SY(n) __oo(cop2d.sh, (n + 12), 1)
#define SZ(n) __oo(cop2d.uh, (n + 17), 0)

#define LIM(a, min, max, bit) \
    (((sw)(a) < min) ? (FLAG |= (bit), min) : \
    (((sw)(a) > max) ? (FLAG |= (bit), max) : ((sw)(a))))

#define SETF(n) \
    (1 << n)

#define limB1(a, l) LIM((a), !l * -32768, 32767, SETF(24) | SETF(31))
#define limB2(a, l) LIM((a), !l * -32768, 32767, SETF(23) | SETF(31))
#define limB3(a, l) LIM((a), !l * -32768, 32767, SETF(22))
#define limC1(a) LIM((a),     0,   255, SETF(21))
#define limC2(a) LIM((a),     0,   255, SETF(20))
#define limC3(a) LIM((a),     0,   255, SETF(19))
#define limD( a) LIM((a),     0, 65535, SETF(18) | SETF(31))
#define limG1(a) LIM((a), -1024,  1023, SETF(14) | SETF(31))
#define limG2(a) LIM((a), -1024,  1023, SETF(13) | SETF(31))
#define limH( a) LIM((a),     0,  4096, SETF(12))

#define _SF(op) ((op >> 19) & 1)
#define _MX(op) ((op >> 17) & 3)
#define _V( op) ((op >> 15) & 3)
#define _CV(op) ((op >> 13) & 3)
#define _LM(op) ((op >> 10) & 1)

#define MAC2IR(n) \
    IR1 = limB1(MAC1, n); \
    IR2 = limB2(MAC2, n); \
    IR3 = limB3(MAC3, n)

#define MAC2RGB4() \
    RGB0  = RGB1; \
    RGB1  = RGB2; \
    CODE2 = CODE; \
    \
    R2 = limC1(MAC1 >> 4); \
    G2 = limC2(MAC2 >> 4); \
    B2 = limC3(MAC3 >> 4)

#define op \
    (code & 0x1ffffff)

// MAC arithmetic overflow flags, checked on the post-shift result:
// F  -> MAC0 overflow, bits 16 (positive) / 15 (negative)
// A1 -> MAC1 overflow, bits 30 / 27
// A2 -> MAC2 overflow, bits 29 / 26
// A3 -> MAC3 overflow, bits 28 / 25
// The MAC1-3 bits mirror into the master error bit 31, MAC0's do not
#define F( a) bounds((a), SETF(16)           , SETF(15))
#define A1(a) bounds((a), SETF(30) | SETF(31), SETF(27) | SETF(31))
#define A2(a) bounds((a), SETF(29) | SETF(31), SETF(26) | SETF(31))
#define A3(a) bounds((a), SETF(28) | SETF(31), SETF(25) | SETF(31))


CstrCop2 cop2;

// Unsigned Newton-Raphson reciprocal table for RTPS/RTPT (257 entries)
static const ub unrTable[257] = {
    0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,0xee,0xec,0xea,0xe8,0xe6,0xe4,0xe3,
    0xe1,0xdf,0xdd,0xdc,0xda,0xd8,0xd6,0xd5,0xd3,0xd1,0xd0,0xce,0xcd,0xcb,0xc9,0xc8,
    0xc6,0xc5,0xc3,0xc1,0xc0,0xbe,0xbd,0xbb,0xba,0xb8,0xb7,0xb5,0xb4,0xb2,0xb1,0xb0,
    0xae,0xad,0xab,0xaa,0xa9,0xa7,0xa6,0xa4,0xa3,0xa2,0xa0,0x9f,0x9e,0x9c,0x9b,0x9a,
    0x99,0x97,0x96,0x95,0x94,0x92,0x91,0x90,0x8f,0x8d,0x8c,0x8b,0x8a,0x89,0x87,0x86,
    0x85,0x84,0x83,0x82,0x81,0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x75,0x74,
    0x73,0x72,0x71,0x70,0x6f,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0x67,0x66,0x65,0x64,
    0x63,0x62,0x61,0x60,0x5f,0x5e,0x5d,0x5d,0x5c,0x5b,0x5a,0x59,0x58,0x57,0x56,0x55,
    0x54,0x53,0x53,0x52,0x51,0x50,0x4f,0x4e,0x4d,0x4d,0x4c,0x4b,0x4a,0x49,0x48,0x48,
    0x47,0x46,0x45,0x44,0x43,0x43,0x42,0x41,0x40,0x3f,0x3f,0x3e,0x3d,0x3c,0x3c,0x3b,
    0x3a,0x39,0x39,0x38,0x37,0x36,0x36,0x35,0x34,0x33,0x33,0x32,0x31,0x31,0x30,0x2f,
    0x2e,0x2e,0x2d,0x2c,0x2c,0x2b,0x2a,0x2a,0x29,0x28,0x28,0x27,0x26,0x26,0x25,0x24,
    0x24,0x23,0x22,0x22,0x21,0x20,0x20,0x1f,0x1e,0x1e,0x1d,0x1d,0x1c,0x1b,0x1b,0x1a,
    0x19,0x19,0x18,0x18,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x12,0x12,0x11,0x11,
    0x10,0x0f,0x0f,0x0e,0x0e,0x0d,0x0d,0x0c,0x0c,0x0b,0x0a,0x0a,0x09,0x09,0x08,0x08,
    0x07,0x07,0x06,0x06,0x05,0x05,0x04,0x04,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,
    0x00
};

void CstrCop2::reset() {
    cop2c = { 0 };
    cop2d = { 0 };
}

sd CstrCop2::bounds(sd value, uw posFlag, uw negFlag) {
    if (value > INT32_MAX) FLAG |= posFlag;
    if (value < INT32_MIN) FLAG |= negFlag;
    return value;
}

uw CstrCop2::limE(uw result) {
    if (result > 131071) {
        FLAG |= SETF(17) | SETF(31);
        return 131071;
    }
    return result;
}

uw CstrCop2::divide(uh n, uh d) {
    // Hardware UNR reciprocal (psx-spx "GTE Division Inaccuracy")
    if ((uw)n < (uw)d * 2u) {
        const uw z = (uw)(__builtin_clz((uw)d) - 16);
        ud nn = (uw)n << z;
        uw dd = (uw)d << z;
        const uw u = unrTable[(dd - 0x7fc0) >> 7] + 0x101;
        dd = ((0x2000080u - (dd * u)) >> 8);
        dd = ((0x0000080u + (dd * u)) >> 8);
        nn = (nn * dd + 0x8000) >> 16;
        return nn > 0x1ffff ? 0x1ffff : (uw)nn;
    }

    FLAG |= SETF(17) | SETF(31);
    return 0x1ffff;
}

void CstrCop2::execute(uw code) {
    switch(code & 0x3f) {
        case 0: // BASIC
            switch(rs & 7) {
                case 0: // MFC2
                    cpu.base[rt] = MFC2(rd);
                    return;

                case 2: // CFC2
                    cpu.base[rt] = oooo(cop2c.uw, rd);
                    return;

                case 4: // MTC2
                    MTC2(rd, cpu.base[rt]);
                    return;

                case 6: // CTC2
                    CTC2(rd, cpu.base[rt]);
                    return;
            }
            
            printx("/// PSeudo GTE Basic %d", (rs & 7));
            return;
            
        case 1: // RTPS
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                sd mac1 = ((sd)TRX << 12) + (R11 * VX0) + (R12 * VY0) + (R13 * VZ0);
                sd mac2 = ((sd)TRY << 12) + (R21 * VX0) + (R22 * VY0) + (R23 * VZ0);
                sd mac3 = ((sd)TRZ << 12) + (R31 * VX0) + (R32 * VY0) + (R33 * VZ0);

                MAC1 = A1(mac1 >> sh);
                MAC2 = A2(mac2 >> sh);
                MAC3 = A3(mac3 >> sh);

                IR1 = limB1(MAC1, lm);
                IR2 = limB2(MAC2, lm);
                // FLAG.22 from (mac3>>12) as lm=0; stored IR3 clamps MAC3 with lm
                (void)limB3((sw)(mac3 >> 12), 0);
                if (lm) {
                    IR3 = MAC3 < 0 ? 0 : (MAC3 > 32767 ? 32767 : (sw)MAC3);
                }
                else {
                    IR3 = MAC3 < -32768 ? -32768 : (MAC3 > 32767 ? 32767 : (sw)MAC3);
                }
                
                SZ0 = SZ1;
                SZ1 = SZ2;
                SZ2 = SZ3;
                SZ3 = limD(mac3 >> 12);
                
                sw quotient = limE(divide(H, SZ3));
                
                SXY0 = SXY1;
                SXY1 = SXY2;
                SX2  = limG1(F((sd)OFX + ((sd)IR1 * quotient)) >> 16);
                SY2  = limG2(F((sd)OFY + ((sd)IR2 * quotient)) >> 16);
                
                MAC0 = F((sd)DQB + ((sd)DQA * quotient));
                IR0 = limH(MAC0 >> 12);
            }
            return;
            
        case 48: // RTPT
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);
                sw quotient = -1;
                
                FLAG = 0;
                SZ0  = SZ3;
                
                for (int v = 0; v < 3; v++) {
                    sw v1 = VX(v);
                    sw v2 = VY(v);
                    sw v3 = VZ(v);
                    
                    sd mac1 = ((sd)TRX << 12) + (R11 * v1) + (R12 * v2) + (R13 * v3);
                    sd mac2 = ((sd)TRY << 12) + (R21 * v1) + (R22 * v2) + (R23 * v3);
                    sd mac3 = ((sd)TRZ << 12) + (R31 * v1) + (R32 * v2) + (R33 * v3);

                    MAC1 = A1(mac1 >> sh);
                    MAC2 = A2(mac2 >> sh);
                    MAC3 = A3(mac3 >> sh);

                    IR1 = limB1(MAC1, lm);
                    IR2 = limB2(MAC2, lm);
                    (void)limB3((sw)(mac3 >> 12), 0);
                    if (lm) {
                        IR3 = MAC3 < 0 ? 0 : (MAC3 > 32767 ? 32767 : (sw)MAC3);
                    }
                    else {
                        IR3 = MAC3 < -32768 ? -32768 : (MAC3 > 32767 ? 32767 : (sw)MAC3);
                    }
                    
                    SZ(v) = limD(mac3 >> 12);
                    quotient = limE(divide(H, SZ(v)));
                    
                    SX(v) = limG1(F((sd)OFX + ((sd)IR1 * quotient)) >> 16);
                    SY(v) = limG2(F((sd)OFY + ((sd)IR2 * quotient)) >> 16);
                }
                
                MAC0 = F((sd)DQB + ((sd)DQA * quotient));
                IR0 = limH(MAC0 >> 12);
            }
            return;
            
        case 6: // NCLIP
            {
                FLAG = 0;
                
                MAC0 = F((sd)SX0 * (SY1 - SY2) + SX1 * (SY2 - SY0) + SX2 * (SY0 - SY1));
            }
            return;
            
        case 12: // OP
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);
                
                FLAG = 0;
                
                MAC1 = A1(((sd)(R22 * IR3) - (R33 * IR2)) >> sh);
                MAC2 = A2(((sd)(R33 * IR1) - (R11 * IR3)) >> sh);
                MAC3 = A3(((sd)(R11 * IR2) - (R22 * IR1)) >> sh);
                
                MAC2IR(lm);
            }
            return;
            
        case 16: // DPCS
            {
                sw sh = _SF(op) * 12;
                
                FLAG = 0;
                
                MAC1 = A1(((R << 16) + (IR0 * limB1(A1((sd)RFC - (R << 4)) << (12 - sh), 0))) >> sh);
                MAC2 = A2(((G << 16) + (IR0 * limB2(A2((sd)GFC - (G << 4)) << (12 - sh), 0))) >> sh);
                MAC3 = A3(((B << 16) + (IR0 * limB3(A3((sd)BFC - (B << 4)) << (12 - sh), 0))) >> sh);
                
                MAC2IR(0);
                
                MAC2RGB4();
            }
            return;
            
        case 42: // DPCT
            {
                sw sh = _SF(op) * 12;

                FLAG = 0;
                
                for (int v = 0; v < 3; v++) {
                    MAC1 = A1((((sd)R0 << 16) + ((sd)IR0 * (limB1(RFC - (R0 << 4), 0)))) >> sh);
                    MAC2 = A2((((sd)G0 << 16) + ((sd)IR0 * (limB2(GFC - (G0 << 4), 0)))) >> sh);
                    MAC3 = A3((((sd)B0 << 16) + ((sd)IR0 * (limB3(BFC - (B0 << 4), 0)))) >> sh);
                    
                    MAC2RGB4();
                }
                
                MAC2IR(0);
            }
            return;
            
        case 17: // INTPL
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);
                
                FLAG = 0;
                
                MAC1 = A1(((IR1 << 12) + (IR0 * limB1(((sd)RFC - IR1), 0))) >> sh);
                MAC2 = A2(((IR2 << 12) + (IR0 * limB2(((sd)GFC - IR2), 0))) >> sh);
                MAC3 = A3(((IR3 << 12) + (IR0 * limB3(((sd)BFC - IR3), 0))) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 18: // MVMVA
            {
                sw sh = _SF(op) * 12;
                sw cv = _CV(op);
                sw mx = _MX(op);
                sw lm = _LM(op);
                sw v  = _V(op);
                sw v1 = VX(v);
                sw v2 = VY(v);
                sw v3 = VZ(v);

                FLAG = 0;

                sw m11, m12, m13, m21, m22, m23, m31, m32, m33;
                if (mx < 3) {
                    m11 = MX11(mx); m12 = MX12(mx); m13 = MX13(mx);
                    m21 = MX21(mx); m22 = MX22(mx); m23 = MX23(mx);
                    m31 = MX31(mx); m32 = MX32(mx); m33 = MX33(mx);
                }
                else {
                    // Garbage matrix: [-R*10h, +R*10h, IR0, R13, R13, R13, R22, R22, R22]
                    const sw r16 = (sw)R * 0x10;
                    m11 = -r16; m12 = r16;  m13 = IR0;
                    m21 = R13;  m22 = R13;  m23 = R13;
                    m31 = R22;  m32 = R22;  m33 = R22;
                }

                if (cv == 2) {
                    // FC translation is bugged: FLAG bits follow the full
                    // formula, but MAC omits Tx and the Mx11*Vx1 term.
                    A1((((sd)RFC << 12) + (m11 * v1) + (m12 * v2) + (m13 * v3)) >> sh);
                    A2((((sd)GFC << 12) + (m21 * v1) + (m22 * v2) + (m23 * v3)) >> sh);
                    A3((((sd)BFC << 12) + (m31 * v1) + (m32 * v2) + (m33 * v3)) >> sh);
                    MAC1 = (sw)(((sd)(m12 * v2) + (m13 * v3)) >> sh);
                    MAC2 = (sw)(((sd)(m22 * v2) + (m23 * v3)) >> sh);
                    MAC3 = (sw)(((sd)(m32 * v2) + (m33 * v3)) >> sh);
                }
                else {
                    MAC1 = A1((((sd)CV1(cv) << 12) + (m11 * v1) + (m12 * v2) + (m13 * v3)) >> sh);
                    MAC2 = A2((((sd)CV2(cv) << 12) + (m21 * v1) + (m22 * v2) + (m23 * v3)) >> sh);
                    MAC3 = A3((((sd)CV3(cv) << 12) + (m31 * v1) + (m32 * v2) + (m33 * v3)) >> sh);
                }

                MAC2IR(lm);
            }
            return;
            
        case 19: // NCDS
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                MAC1 = A1((((sd)L11 * VX0) + (L12 * VY0) + (L13 * VZ0)) >> sh);
                MAC2 = A2((((sd)L21 * VX0) + (L22 * VY0) + (L23 * VZ0)) >> sh);
                MAC3 = A3((((sd)L31 * VX0) + (L32 * VY0) + (L33 * VZ0)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1(((((sd)R << 4) * IR1) + (IR0 * limB1(RFC - ((R * IR1) >> 8), 0))) >> sh);
                MAC2 = A2(((((sd)G << 4) * IR2) + (IR0 * limB2(GFC - ((G * IR2) >> 8), 0))) >> sh);
                MAC3 = A3(((((sd)B << 4) * IR3) + (IR0 * limB3(BFC - ((B * IR3) >> 8), 0))) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 22: // NCDT
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                for (int v = 0; v < 3; v++) {
                    sw v1 = VX(v);
                    sw v2 = VY(v);
                    sw v3 = VZ(v);
                    
                    MAC1 = A1((((sd)L11 * v1) + (L12 * v2) + (L13 * v3)) >> sh);
                    MAC2 = A2((((sd)L21 * v1) + (L22 * v2) + (L23 * v3)) >> sh);
                    MAC3 = A3((((sd)L31 * v1) + (L32 * v2) + (L33 * v3)) >> sh);
                    
                    MAC2IR(lm);
                    
                    MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                    MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                    MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                    
                    MAC2IR(lm);
                    
                    MAC1 = A1(((((sd)R << 4) * IR1) + (IR0 * limB1(RFC - ((R * IR1) >> 8), 0))) >> sh);
                    MAC2 = A2(((((sd)G << 4) * IR2) + (IR0 * limB2(GFC - ((G * IR2) >> 8), 0))) >> sh);
                    MAC3 = A3(((((sd)B << 4) * IR3) + (IR0 * limB3(BFC - ((B * IR3) >> 8), 0))) >> sh);
                    
                    MAC2RGB4();
                }
                
                MAC2IR(lm);
            }
            return;
            
        case 20: // CDP
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1(((((sd)R << 4) * IR1) + (IR0 * limB1(RFC - ((R * IR1) >> 8), 0))) >> sh);
                MAC2 = A2(((((sd)G << 4) * IR2) + (IR0 * limB2(GFC - ((G * IR2) >> 8), 0))) >> sh);
                MAC3 = A3(((((sd)B << 4) * IR3) + (IR0 * limB3(BFC - ((B * IR3) >> 8), 0))) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 27: // NCCS
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                MAC1 = A1((((sd)L11 * VX0) + (L12 * VY0) + (L13 * VZ0)) >> sh);
                MAC2 = A2((((sd)L21 * VX0) + (L22 * VY0) + (L23 * VZ0)) >> sh);
                MAC3 = A3((((sd)L31 * VX0) + (L32 * VY0) + (L33 * VZ0)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1((((sd)R * IR1) << 4) >> sh);
                MAC2 = A2((((sd)G * IR2) << 4) >> sh);
                MAC3 = A3((((sd)B * IR3) << 4) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 63: // NCCT
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                for (int v = 0; v < 3; v++) {
                    sw v1 = VX(v);
                    sw v2 = VY(v);
                    sw v3 = VZ(v);
                    
                    MAC1 = A1((((sd)L11 * v1) + (L12 * v2) + (L13 * v3)) >> sh);
                    MAC2 = A2((((sd)L21 * v1) + (L22 * v2) + (L23 * v3)) >> sh);
                    MAC3 = A3((((sd)L31 * v1) + (L32 * v2) + (L33 * v3)) >> sh);
                    
                    MAC2IR(lm);
                    
                    MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                    MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                    MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                    
                    MAC2IR(lm);
                    
                    MAC1 = A1((((sd)R * IR1) << 4) >> sh);
                    MAC2 = A2((((sd)G * IR2) << 4) >> sh);
                    MAC3 = A3((((sd)B * IR3) << 4) >> sh);
                    
                    MAC2RGB4();
                }
                
                MAC2IR(lm);
            }
            return;
            
        case 28: // CC
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1((((sd)R * IR1) << 4) >> sh);
                MAC2 = A2((((sd)G * IR2) << 4) >> sh);
                MAC3 = A3((((sd)B * IR3) << 4) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 30: // NCS
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                MAC1 = A1((((sd)L11 * VX0) + (L12 * VY0) + (L13 * VZ0)) >> sh);
                MAC2 = A2((((sd)L21 * VX0) + (L22 * VY0) + (L23 * VZ0)) >> sh);
                MAC3 = A3((((sd)L31 * VX0) + (L32 * VY0) + (L33 * VZ0)) >> sh);
                
                MAC2IR(lm);
                
                MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 32: // NCT
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);

                FLAG = 0;
                
                for (int v = 0; v < 3; v++) {
                    sw v1 = VX(v);
                    sw v2 = VY(v);
                    sw v3 = VZ(v);
                    
                    MAC1 = A1((((sd)L11 * v1) + (L12 * v2) + (L13 * v3)) >> sh);
                    MAC2 = A2((((sd)L21 * v1) + (L22 * v2) + (L23 * v3)) >> sh);
                    MAC3 = A3((((sd)L31 * v1) + (L32 * v2) + (L33 * v3)) >> sh);
                    
                    MAC2IR(lm);
                    
                    MAC1 = A1((((sd)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3)) >> sh);
                    MAC2 = A2((((sd)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3)) >> sh);
                    MAC3 = A3((((sd)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3)) >> sh);
                    
                    MAC2RGB4();
                }
                
                MAC2IR(lm);
            }
            return;
            
        case 40: // SQR
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);
                
                FLAG = 0;
                
                MAC1 = A1((IR1 * IR1) >> sh);
                MAC2 = A2((IR2 * IR2) >> sh);
                MAC3 = A3((IR3 * IR3) >> sh);
                
                MAC2IR(lm);
            }
            return;
            
        case 41: // DCPL
            {
                sw sh = _SF(op) * 12;
                sw lm = _LM(op);
                
                sd RIR1 = ((sd)R * IR1) << 4;
                sd GIR2 = ((sd)G * IR2) << 4;
                sd BIR3 = ((sd)B * IR3) << 4;
                
                FLAG = 0;
                
                MAC1 = A1((RIR1 + (IR0 * limB1(RFC - (RIR1 >> 12), 0))) >> sh);
                MAC2 = A2((GIR2 + (IR0 * limB2(GFC - (GIR2 >> 12), 0))) >> sh);
                MAC3 = A3((BIR3 + (IR0 * limB3(BFC - (BIR3 >> 12), 0))) >> sh);
                
                MAC2IR(lm);
                
                MAC2RGB4();
            }
            return;
            
        case 45: // AVSZ3
            {
                FLAG = 0;
                
                MAC0 = F((sd)(ZSF3 * SZ1) + (ZSF3 * SZ2) + (ZSF3 * SZ3));
                OTZ = limD(MAC0 >> 12);
            }
            return;

        case 46: // AVSZ4
            {
                FLAG = 0;
                
                MAC0 = F((sd)(ZSF4 * (SZ0 + SZ1 + SZ2 + SZ3)));
                OTZ = limD(MAC0 >> 12);
            }
            return;
            
        case 61: // GPF
            {
                sw sh = _SF(op) * 12;
                
                FLAG = 0;
                
                MAC1 = A1(((sd)IR0 * IR1) >> sh);
                MAC2 = A2(((sd)IR0 * IR2) >> sh);
                MAC3 = A3(((sd)IR0 * IR3) >> sh);
                
                MAC2IR(0);
                
                MAC2RGB4();
            }
            return;
            
        case 62: // GPL
            {
                sw sh = _SF(op) * 12;
                
                FLAG = 0;
                
                MAC1 = A1((((sd)MAC1 << sh) + (IR0 * IR1)) >> sh);
                MAC2 = A2((((sd)MAC2 << sh) + (IR0 * IR2)) >> sh);
                MAC3 = A3((((sd)MAC3 << sh) + (IR0 * IR3)) >> sh);
                
                MAC2IR(0);
                
                MAC2RGB4();
            }
            return;
    }
    
    printx("/// PSeudo GTE Execute %d", (code & 0x3f));
}

uw CstrCop2::MFC2(uw addr) { // Cop2d read
    switch(addr) {
        case  1: // V0(z)
        case  3: // V1(z)
        case  5: // V2(z)
        case  8: // IR0
        case  9: // IR1
        case 10: // IR2
        case 11: // IR3
            oooo(cop2d.sw, addr) = __oo(cop2d.sh, addr, 0);
            break;
            
        case  7: // OTZ
        case 16: // SZ0
        case 17: // SZ1
        case 18: // SZ2
        case 19: // SZ3
            oooo(cop2d.uw, addr) = __oo(cop2d.uh, addr, 0);
            break;
            
        case 15: // SXY2
            oooo(cop2d.uw, addr) = SXY2;
            break;
            
        case 28: // IRGB
        case 29: // ORGB
            oooo(cop2d.uw, addr) = LIM(IR1 >> 7, 0, 0x1f, 0) | (LIM(IR2 >> 7, 0, 0x1f, 0) << 5) | (LIM(IR3 >> 7, 0, 0x1f, 0) << 10);
            break;
    }
    
    return oooo(cop2d.uw, addr);
}

void CstrCop2::MTC2(uw addr, uw data) { // Cop2d write
    switch(addr) {
        case 15: // SXY3
            SXY0 = SXY1;
            SXY1 = SXY2;
            SXY2 = data;
            SXYP = data;
            return;
            
        case 28: // IRGB
            IRGB = (data);
            IR1  = (data & 0x1f) << 7;
            IR2  = (data & 0x3e0) << 2;
            IR3  = (data & 0x7c00) >> 3;
            return;
            
        case 30: // LZCS
            {
                int a;
                LZCS = data;
                
                a = LZCS;
                if (a > 0) {
                    int i;
                    for (i = 31; (a & (1 << i)) == 0 && i >= 0; i--);
                    LZCR = 31 - i;
                } else if (a < 0) {
                    int i;
                    a ^= 0xffffffff;
                    for (i = 31; (a & (1 << i)) == 0 && i >= 0; i--);
                    LZCR = 31 - i;
                } else {
                    LZCR = 32;
                }
            }
            return;
            
        case 29: // ORGB
        case 31: // LZCR
            return;
    }
    
    oooo(cop2d.uw, addr) = data;
}

void CstrCop2::CTC2(uw addr, uw data) { // Cop2c write
    switch(addr) {
        case  4: // RT33
        case 12: // L33
        case 20: // LR33
        case 26: // H
        case 27: // DQA
        case 29: // ZSF3
        case 30: // ZSF4
            data = (sw)(sh)data;
            break;
            
        /* unused */
        case 31: // FLAG
            printx("/// PSeudo GTE %s", "FLAG");
            break;
    }
    
    oooo(cop2c.uw, addr) = data;
}
