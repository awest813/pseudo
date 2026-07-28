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

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
