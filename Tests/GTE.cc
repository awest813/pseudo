/* GTE unit tests. Built on the host against the stubbed KOS/GLdc/ALdc
 * headers (see Platforms/Dreamcast/HostCheck), so the whole emulator
 * core links but nothing needs a BIOS or a display. Exercises the
 * register interface and a sample of commands with hand-computed
 * expectations, including the FLAG register semantics. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) { failed++; }
}

// Read the FLAG control register back through the CFC2 path
static uw gteFlag() {
    cpu.base[1] = 0;
    cop2.execute((2 << 21) | (1 << 16) | (31 << 11)); // CFC2 r1, cop2c[31]
    return cpu.base[1];
}

static uw pack(sh x, sh y) {
    return ((uw)(uh)y << 16) | (uh)x;
}

int main() {
    // --- RTPS: identity rotation, H / SZ3 = 1.0 ------------------------
    // V0 = (0x10, 0x20, 0x100), R = I (1.0 = 0x1000 in 4.12), H = 0x100:
    // projects to SXY2 = (0x10, 0x20) with SZ3 = 0x100 and no flags
    cop2.reset();
    cop2.CTC2( 0, 0x1000); // R11 = 1.0, R12 = 0
    cop2.CTC2( 2, 0x1000); // R22 = 1.0, R23 = 0
    cop2.CTC2( 4, 0x1000); // R33 = 1.0
    cop2.CTC2(26, 0x100);  // H
    cop2.MTC2( 0, pack(0x10, 0x20)); // VX0, VY0
    cop2.MTC2( 1, 0x100);            // VZ0
    cop2.execute(0x01);

    check(cop2.MFC2(14) == pack(0x10, 0x20), "RTPS: SXY2 projected");
    check(cop2.MFC2(19) == 0x100,            "RTPS: SZ3 pushed");
    check(cop2.MFC2( 9) == 0x10,             "RTPS: IR1");
    check(gteFlag()     == 0,                "RTPS: no flags raised");

    // --- RTPS: MAC1 positive overflow -> FLAG bits 30 + 31 -------------
    // TRX<<12 + R11*VX0 = 0x7fffffff000 + 0x3fff0001, >>12 exceeds 2^31-1
    cop2.reset();
    cop2.CTC2(5, 0x7fffffff); // TRX
    cop2.CTC2(0, 0x7fff);     // R11
    cop2.MTC2(0, pack(0x7fff, 0));
    cop2.execute(0x01);

    check((gteFlag() & (1u << 30)) != 0, "RTPS: MAC1 positive overflow (bit 30)");
    check((gteFlag() & (1u << 31)) != 0, "RTPS: master error mirror (bit 31)");

    // --- RTPS: MAC1 negative overflow -> FLAG bit 27 --------------------
    cop2.reset();
    cop2.CTC2(5, 0x80000000);       // TRX = -2^31
    cop2.CTC2(0, 0x7fff);           // R11
    cop2.MTC2(0, pack(-0x8000, 0)); // VX0 = -32768
    cop2.execute(0x01);

    check((gteFlag() & (1u << 27)) != 0, "RTPS: MAC1 negative overflow (bit 27)");

    // --- NCLIP: winding + MAC0 overflow -> FLAG bit 16, not bit 31 ------
    // MAC0 = SX0*(SY1-SY2) + SX1*(SY2-SY0) + SX2*(SY0-SY1) = 4294836225
    cop2.reset();
    cop2.MTC2(12, pack( 32767, -32768)); // SXY0
    cop2.MTC2(13, pack( 32767,  32767)); // SXY1
    cop2.MTC2(14, pack(-32768,      0)); // SXY2
    cop2.execute(0x06);

    check((gteFlag() & (1u << 16)) != 0, "NCLIP: MAC0 positive overflow (bit 16)");
    check((gteFlag() & (1u << 31)) == 0, "NCLIP: MAC0 does not mirror to bit 31");

    // NCLIP regression: plain clockwise winding, no flags
    cop2.reset();
    cop2.MTC2(12, pack( 0,  0));
    cop2.MTC2(13, pack(10,  0));
    cop2.MTC2(14, pack( 0, 10));
    cop2.execute(0x06);

    check(cop2.MFC2(24) == 100, "NCLIP: MAC0 winding area");
    check(gteFlag()     == 0,   "NCLIP: no flags raised");

    // --- AVSZ3: ZSF3 * (SZ1 + SZ2 + SZ3) --------------------------------
    cop2.reset();
    cop2.CTC2(29, 0x155);  // ZSF3
    cop2.MTC2(17, 0x1000); // SZ1
    cop2.MTC2(18, 0x1000); // SZ2
    cop2.MTC2(19, 0x1000); // SZ3
    cop2.execute(0x2d);

    check(cop2.MFC2(7) == 0x3ff, "AVSZ3: OTZ average");

    // --- GPF (sf=1): IRn = IR0 * IRn >> 12, RGB2 pushed ------------------
    cop2.reset();
    cop2.MTC2( 8, 0x1000); // IR0
    cop2.MTC2( 9, 0x800);  // IR1
    cop2.MTC2(10, 0x400);  // IR2
    cop2.MTC2(11, 0x200);  // IR3
    cop2.execute(0x3d | (1 << 19));

    check(cop2.MFC2(25) == 0x800,      "GPF: MAC1");
    check(cop2.MFC2(22) == 0x00204080, "GPF: RGB2 color push");

    // --- DCPL: far-color saturation flags are per channel ----------------
    // RFC = 0 (no saturation), GFC / BFC huge: the G interpolation must
    // raise the IR2 flag (bit 23) and B the IR3 flag (bit 22) -- not R's
    cop2.reset();
    cop2.MTC2( 6, 0x00ffff00); // RGB: R = 0, G = 0xff, B = 0xff
    cop2.MTC2( 8, 0x1000);     // IR0
    cop2.MTC2(10, 0x100);      // IR2
    cop2.MTC2(11, 0x100);      // IR3
    cop2.CTC2(22, 0x40000000); // GFC
    cop2.CTC2(23, 0x40000000); // BFC
    cop2.execute(0x29);

    check((gteFlag() & (1u << 23)) != 0, "DCPL: G saturation raises IR2 flag (bit 23)");
    check((gteFlag() & (1u << 22)) != 0, "DCPL: B saturation raises IR3 flag (bit 22)");
    check((gteFlag() & (1u << 24)) == 0, "DCPL: R channel flag (bit 24) untouched");

    // --- LZCS / LZCR leading bit count -----------------------------------
    cop2.MTC2(30, 1);
    check(cop2.MFC2(31) == 31, "LZCR: leading zero count of 1");
    cop2.MTC2(30, 0xffffffff);
    check(cop2.MFC2(31) == 32, "LZCR: leading one count of -1");
    cop2.MTC2(30, 0);
    check(cop2.MFC2(31) == 32, "LZCR: count of 0");

    printf("\n%s (%d failure%s)\n", failed ? "FAILED" : "PASSED", failed, failed == 1 ? "" : "s");
    return failed ? 1 : 0;
}
