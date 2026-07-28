/* GPU unit tests: mask-bit (STP) handling via Graphics::setMask. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

enum {
    GPU_STAT_MASKDRAWN      = 0x00000800,
    GPU_STAT_MASKENABLED    = 0x00001000,
    GPU_STAT_DRAWINGALLOWED = 0x00000400,
};

int main() {
    vs.reset();

  {
    vs.setMask(0x1);
    const uw stat = vs.read(0x1f801814);
    check((stat & GPU_STAT_MASKDRAWN) != 0,   "setMask bit0: MASKDRAWN");
    check((stat & GPU_STAT_MASKENABLED) == 0, "setMask bit0: MASKENABLED clear");
  }

  {
    vs.setMask(0x2);
    const uw stat = vs.read(0x1f801814);
    check((stat & GPU_STAT_MASKENABLED) != 0, "setMask bit1: MASKENABLED");
    check((stat & GPU_STAT_MASKDRAWN) == 0,    "setMask bit1: MASKDRAWN clear");
  }

  {
    vs.setMask(0x3);
    const uw stat = vs.read(0x1f801814);
    check((stat & GPU_STAT_MASKDRAWN) != 0,   "setMask bits0+1: MASKDRAWN");
    check((stat & GPU_STAT_MASKENABLED) != 0,  "setMask bits0+1: MASKENABLED");
  }

  {
    vs.setMask(0x4);
    const uw stat = vs.read(0x1f801814);
    check((stat & GPU_STAT_DRAWINGALLOWED) == 0, "setMask bit2 set: no DRAWINGALLOWED");
  }

  {
    vs.setMask(0x0);
    const uw stat = vs.read(0x1f801814);
    check((stat & GPU_STAT_DRAWINGALLOWED) != 0, "setMask bit2 clear: DRAWINGALLOWED");
  }

  {
    vs.write(0x1f801814, 0x10000000); // info query: GPU type
    check(vs.read(0x1f801810) == 2, "GPU info 0x00: type");
  }

  {
    vs.write(0x1f801814, 0x10000001); // info query: unknown
    check(vs.read(0x1f801810) == 1, "GPU info 0x01");
  }

  {
    vs.write(0x1f801814, 0x10000007); // info query: GPU version
    check(vs.read(0x1f801810) == 2, "GPU info 0x07: version");
  }

  // --- Mask bit enforced on VRAM copy ------------------------------------
  {
    vs.reset();
    vs.vram.ptr[0] = 0x1234;             // source
    vs.vram.ptr[(1 << 10) + 0] = 0x0001; // dest unlocked

    uw packets[4] = {
        0x80000000,
        0x00000000,       // src (0,0)
        0x00010000,       // dst (0,1)
        0x00010001,       // 1x1
    };

    vs.setMask(0x1); // force bit15 on writes
    vs.photoMoveWithin(packets);
    check(vs.vram.ptr[(1 << 10) + 0] == 0x9234, "mask force: bit15 set on write");
  }

  {
    vs.reset();
    vs.vram.ptr[0] = 0x00aa;
    vs.vram.ptr[(1 << 10) + 0] = 0x0001; // unlocked
    vs.vram.ptr[(1 << 10) + 1] = 0x8002; // locked

    uw packets[4] = {
        0x80000000,
        0x00000000,       // src (0,0)
        0x00010000,       // dst (0,1)
        0x00010002,       // 2x1
    };

    vs.setMask(0x2); // skip locked dest
    vs.photoMoveWithin(packets);
    check(vs.vram.ptr[(1 << 10) + 0] == 0x00aa, "mask check: unlocked dest written");
    check(vs.vram.ptr[(1 << 10) + 1] == 0x8002, "mask check: locked dest preserved");
  }

  // --- GP0(02h) VRAM fill ------------------------------------------------
  {
    vs.reset();
    // Seed with non-zero so we can see the clear
    for (int i = 0; i < 64; i++) {
        vs.vram.ptr[i] = 0xffff;
    }

    uw fill[3] = {
        0x020000f8,       // cmd + R=0xf8 G=0 B=0 → R5=0x1f
        0x00000000,       // pos (0,0)
        0x00010010,       // 16x1 (already aligned)
    };
    vs.setMask(0x3); // mask force+check must be ignored by fill
    vs.photoFill(fill);

    check(vs.vram.ptr[0] == 0x001f, "fill: RGB555 red pixel");
    check((vs.vram.ptr[0] & 0x8000) == 0, "fill: bit15 forced clear");
    check(vs.vram.ptr[15] == 0x001f, "fill: covers width");
    check(vs.vram.ptr[16] == 0xffff, "fill: stops at width");

    // Xpos rounded down to 0x10 steps; Xsiz rounded up
    uw fill2[3] = {
        0x02f80000,       // B=0xf8 → bits 10-14
        0x00000005,       // X=5 → rounded to 0
        0x00010001,       // W=1 → rounded to 0x10
    };
    vs.photoFill(fill2);
    check(vs.vram.ptr[0] == 0x7c00, "fill: rounding still fills 16px");

    // Zero size: no write
    vs.vram.ptr[0] = 0x1234;
    uw fill0[3] = { 0x020000ff, 0, 0x00010000 };
    vs.photoFill(fill0);
    check(vs.vram.ptr[0] == 0x1234, "fill: Xsiz=0 is no-op");
  }

  // --- GP0(01h) Clear Cache via primitive --------------------------------
  {
    uw cmd = 0x01000000;
    draw.primitive(0x01, &cmd); // must not abort / printx
    check(true, "clear cache: primitive accepted");
  }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
