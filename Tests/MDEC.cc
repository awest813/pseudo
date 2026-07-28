/* MDEC status / command protocol unit tests. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

enum {
    STAT_FIFO_EMPTY  = 0x80000000,
    STAT_BUSY        = 0x20000000,
    STAT_DIN_REQ     = 0x10000000,
    STAT_DOUT_REQ    = 0x08000000,
    STAT_DEPTH_MASK  = 0x06000000,
    STAT_BIT15       = 0x00800000,
};

int main() {
    mem.reset();
    mdec.reset();

  {
    const uw st = mdec.read(0x1f801824);
    check((st & STAT_FIFO_EMPTY) != 0, "reset: fifo empty");
    check((st & STAT_BUSY) == 0, "reset: not busy");
    check((st & 0x00070000) == 0x00040000, "reset: current block Cr");
  }

  {
    // Decode 15-bit with bit15 set, 4 parameter words
    mdec.write(0x1f801820, 0x38000004 | 0x02000000);
    const uw st = mdec.read(0x1f801824);
    check((st & STAT_BUSY) != 0, "decode: busy");
    check((st & 0xffff) == 3, "decode: param words minus 1");
    check((st & STAT_DEPTH_MASK) == 0x06000000, "decode: 15-bit depth");
    check((st & STAT_BIT15) != 0, "decode: bit15 mirrored");
  }

  {
    mdec.write(0x1f801824, 0x40000000); // enable data-in
    const uw st = mdec.read(0x1f801824);
    check((st & STAT_DIN_REQ) != 0, "control: data-in request");
  }

  {
    // Quant color tables: 32 words
    mdec.reset();
    mdec.write(0x1f801820, 0x40000001);
    const uw st = mdec.read(0x1f801824);
    check((st & STAT_BUSY) != 0, "quant color: busy");
    check((st & 0xffff) == 31, "quant color: 32 words minus 1");
  }

  {
    // Quant luminance only: 16 words
    mdec.reset();
    mdec.write(0x1f801820, 0x40000000);
    const uw st = mdec.read(0x1f801824);
    check((st & 0xffff) == 15, "quant mono: 16 words minus 1");
  }

  {
    // Scale table: 32 words
    mdec.reset();
    mdec.write(0x1f801820, 0x60000000);
    const uw st = mdec.read(0x1f801824);
    check((st & STAT_BUSY) != 0, "scale: busy");
    check((st & 0xffff) == 31, "scale: 32 words minus 1");

    ub iq[128];
    memset(iq, 1, sizeof(iq));
    memcpy(&mem.ram.ptr[0], iq, sizeof(iq));

    CstrBus::castDMA dma = { 0 };
    dma.madr = 0;
    dma.bcr  = 0x00010020; // 1 block * 32 words
    dma.chcr = 0x01000201;
    mdec.executeDMA(&dma);

    const uw after = mdec.read(0x1f801824);
    check((after & STAT_BUSY) == 0, "scale DMA: busy cleared");
    check((after & 0xffff) == 0xffff, "scale DMA: no params remaining");
  }

  {
    // Quant + decode smoke: feed constant IQ and a padded empty-ish RL stream
    mdec.reset();
    mdec.write(0x1f801824, 0x60000000); // enable in+out

    ub tables[128];
    memset(tables, 16, sizeof(tables));
    memcpy(&mem.ram.ptr[0x1000], tables, sizeof(tables));

    mdec.write(0x1f801820, 0x40000001);
    CstrBus::castDMA din = { 0 };
    din.madr = 0x1000;
    din.bcr  = 0x00010020;
    din.chcr = 0x01000201;
    mdec.executeDMA(&din);
    check((mdec.read(0x1f801824) & STAT_BUSY) == 0, "quant DMA: done");

    // Build a minimal colored macroblock: 6 blocks of (DC=0, EOB)
    uh *rl = (uh *)&mem.ram.ptr[0x2000];
    for (int i = 0; i < 6; i++) {
        *rl++ = 0x0000; // q_scale=0, DC=0
        *rl++ = 0xfe00; // EOB
    }

    mdec.write(0x1f801820, 0x38000010); // 15-bit decode, 16 words
    din.madr = 0x2000;
    din.bcr  = 0x00010010;
    din.chcr = 0x01000201;
    mdec.executeDMA(&din);

    uw st = mdec.read(0x1f801824);
    check((st & STAT_DOUT_REQ) != 0, "decode: data-out request");
    check((st & STAT_BUSY) == 0, "decode DMA in: busy cleared");

    CstrBus::castDMA dout = { 0 };
    dout.madr = 0x3000;
    dout.bcr  = 0x00010080; // 128 words = 256 halfwords (one 15-bit MB)
    dout.chcr = 0x01000200;
    mdec.executeDMA(&dout);

    st = mdec.read(0x1f801824);
    check((st & STAT_DOUT_REQ) == 0, "decode DMA out: request cleared");
    check((st & STAT_FIFO_EMPTY) != 0, "decode DMA out: fifo empty");

    // First pixel: neutral chroma → R and B channels match (PSX BGR555)
    const uh pixel = *(uh *)&mem.ram.ptr[0x3000];
    const int r = pixel & 0x1f;
    const int b = (pixel >> 10) & 0x1f;
    check(r == b, "decode: neutral chroma R==B");
  }

  {
    mdec.write(0x1f801824, 0x80000000);
    const uw st = mdec.read(0x1f801824);
    check(st == 0x80040000, "control reset: status 0x80040000");
  }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
