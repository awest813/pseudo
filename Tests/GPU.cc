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

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
