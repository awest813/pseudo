/* GPU texture-window (GP0 E2) unit tests. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

int main() {
    draw.reset();

  {
    // Mask=0 → identity mapping (full 256x256 page)
    draw.setTexWindow(0);
    check(draw.mapTexU(0x00) == 0x00, "identity U 0");
    check(draw.mapTexU(0xff) == 0xff, "identity U 255");
    check(draw.mapTexV(0x80) == 0x80, "identity V 128");
    check(draw.texWindowRaw() == 0,   "raw window 0");
  }

  {
    // 8x8 window at (0,0): MaskX/Y = 0x1f → keep only low 3 bits
    // cmd = maskX | (maskY<<5) = 0x1f | (0x1f<<5) = 0x3ff
    draw.setTexWindow(0x3ff);
    check(draw.mapTexU(0x00) == 0x00, "8x8: U 0 -> 0");
    check(draw.mapTexU(0x07) == 0x07, "8x8: U 7 -> 7");
    check(draw.mapTexU(0x08) == 0x00, "8x8: U 8 -> 0");
    check(draw.mapTexU(0x1f) == 0x07, "8x8: U 31 -> 7");
    check(draw.mapTexV(0x10) == 0x00, "8x8: V 16 -> 0");
  }

  {
    // 8x8 window offset to (16,24):
    // OffsetX=2 (16/8), OffsetY=3 (24/8), Mask=0x1f
    // cmd = 0x1f | (0x1f<<5) | (2<<10) | (3<<15)
    const uw cmd = 0x1f | (0x1f << 5) | (2 << 10) | (3 << 15);
    draw.setTexWindow(cmd);
    check(draw.mapTexU(0x00) == 16, "8x8@16: U 0 -> 16");
    check(draw.mapTexU(0x05) == 21, "8x8@16: U 5 -> 21");
    check(draw.mapTexU(0x08) == 16, "8x8@16: U 8 -> 16");
    check(draw.mapTexV(0x00) == 24, "8x8@24: V 0 -> 24");
    check(draw.mapTexV(0x07) == 31, "8x8@24: V 7 -> 31");
    check(draw.texWindowRaw() == (cmd & 0xfffff), "raw window stored");
  }

  {
    // 16x16 tiling: Mask = 0x1e (bits that clear every 16)
    // Mask*8 = 0xF0, keep low 4 bits
    draw.setTexWindow(0x1e | (0x1e << 5));
    check(draw.mapTexU(0x00) == 0x00, "16x16: U 0 -> 0");
    check(draw.mapTexU(0x0f) == 0x0f, "16x16: U 15 -> 15");
    check(draw.mapTexU(0x10) == 0x00, "16x16: U 16 -> 0");
    check(draw.mapTexU(0x1f) == 0x0f, "16x16: U 31 -> 15");
  }

  {
    // GP0(E2) via primitive path mirrors into GPU info register
    uw packet = 0xe2000000 | 0x3ff;
    draw.primitive(0xe2, &packet);
    check(vs.info[vs.GPU_INFO_TEX_WINDOW] == 0x3ff, "E2 primitive: info register");
    check(draw.mapTexU(0x09) == 0x01, "E2 primitive: mapping active");
  }

  {
    vs.write(0x1f801814, 0x10000002); // info query: texture window
    check(vs.read(0x1f801810) == 0x3ff, "GP1(10h) info 02h: tex window");
  }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
