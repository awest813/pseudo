/* R3000A CPU unit tests. Drives real instructions through the public
 * interface: a small MIPS program is written into emulated RAM and run
 * with bootstrap(), which single-steps until pc reaches 0x80030000.
 * Each program ends with `J 0x80030000` + a NOP delay slot as the
 * sentinel, so results land in the general registers where the test
 * can read them back. Links the whole core against the Dreamcast
 * HostCheck stubs, so no BIOS or display is needed. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) { failed++; }
}

// MIPS instruction encoders (only the forms these tests need).
// Note: R3000A.h defines rs/rt/rd/sa/imm as macros, so the parameters
// here use short neutral names to avoid colliding with them.
static uw rType(uw fn, uw s, uw t, uw d, uw shamt) {
    return (s << 21) | (t << 16) | (d << 11) | (shamt << 6) | fn;
}
static uw iType(uw op, uw s, uw t, uh im) {
    return (op << 26) | (s << 21) | (t << 16) | im;
}

// Load a program at 0x80010000 and run it to the 0x80030000 sentinel.
// The caller supplies everything except the trailing jump + delay slot.
static const uw PROG_BASE  = 0x80010000;
static const uw PROG_END   = 0x80030000;

static void runProgram(const uw *body, int count) {
    cpu.setpc(PROG_BASE);
    uw *prog = (uw *)&mem.ram.ptr[PROG_BASE & (mem.ram.size - 1)];

    int i = 0;
    for (; i < count; i++) {
        prog[i] = body[i];
    }
    // J 0x80030000  (target = (0x80030000 & 0x0fffffff) >> 2)
    prog[i++] = iType(2, 0, 0, 0) | ((PROG_END & 0x0fffffff) >> 2);
    prog[i++] = 0; // NOP delay slot

    cpu.bootstrap();
}

// funct codes
enum { SLL = 0, DIV = 26, DIVU = 27, MFHI = 16, MFLO = 18, ADDU = 33, OR = 37 };

int main() {
    // --- DIV: INT_MIN / -1 overflow (the crash case) --------------------
    // C++ signed overflow here is UB and traps (#DE / SIGFPE) on x86;
    // the R3000 defines lo = 0x80000000, hi = 0
    {
        cpu.reset();
        cpu.base[8] = 0x80000000; // $t0 dividend
        cpu.base[9] = 0xffffffff; // $t1 divisor (-1)

        uw body[] = {
            rType(DIV,  8, 9, 0,  0),  // div  $t0, $t1
            rType(MFLO, 0, 0, 10, 0),  // mflo $t2
            rType(MFHI, 0, 0, 11, 0),  // mfhi $t3
        };
        runProgram(body, 3);

        check(cpu.base[10] == 0x80000000, "DIV: INT_MIN / -1 -> lo = 0x80000000");
        check(cpu.base[11] == 0,          "DIV: INT_MIN / -1 -> hi = 0");
    }

    // --- DIV: ordinary signed division ----------------------------------
    {
        cpu.reset();
        cpu.base[8] = (uw)-7;
        cpu.base[9] = 2;

        uw body[] = {
            rType(DIV,  8, 9, 0,  0),
            rType(MFLO, 0, 0, 10, 0),
            rType(MFHI, 0, 0, 11, 0),
        };
        runProgram(body, 3);

        check((sw)cpu.base[10] == -3, "DIV: -7 / 2 -> lo = -3 (truncated)");
        check((sw)cpu.base[11] == -1, "DIV: -7 %% 2 -> hi = -1");
    }

    // --- DIV by zero: hardware-defined result ---------------------------
    {
        cpu.reset();
        cpu.base[8] = 5;      // positive dividend
        cpu.base[9] = 0;

        uw body[] = {
            rType(DIV,  8, 9, 0,  0),
            rType(MFLO, 0, 0, 10, 0),
            rType(MFHI, 0, 0, 11, 0),
        };
        runProgram(body, 3);

        check(cpu.base[10] == 0xffffffff, "DIV/0: positive -> lo = -1");
        check(cpu.base[11] == 5,          "DIV/0: hi = dividend");
    }

    // --- DIVU by zero ---------------------------------------------------
    {
        cpu.reset();
        cpu.base[8] = 12345;
        cpu.base[9] = 0;

        uw body[] = {
            rType(DIVU, 8, 9, 0,  0),
            rType(MFLO, 0, 0, 10, 0),
            rType(MFHI, 0, 0, 11, 0),
        };
        runProgram(body, 3);

        check(cpu.base[10] == 0xffffffff, "DIVU/0: lo = 0xffffffff");
        check(cpu.base[11] == 12345,      "DIVU/0: hi = dividend");
    }

    // --- ADDU / OR / SLL sanity + $zero stays zero ----------------------
    {
        cpu.reset();
        cpu.base[8] = 0x0000ffff;
        cpu.base[9] = 0xffff0000;

        uw body[] = {
            rType(OR,   8, 9, 10, 0),  // or   $t2, $t0, $t1 -> 0xffffffff
            rType(SLL,  0, 10, 11, 4), // sll  $t3, $t2, 4   -> 0xfffffff0
            rType(ADDU, 8, 9, 12, 0),  // addu $t4, $t0, $t1 -> 0xffffffff
            iType(13,   0, 0,  0x1234), // ori  $zero, $zero, 0x1234 (must stay 0)
        };
        runProgram(body, 4);

        check(cpu.base[10] == 0xffffffff, "OR: bitwise or");
        check(cpu.base[11] == 0xfffffff0, "SLL: shift left by 4");
        check(cpu.base[12] == 0xffffffff, "ADDU: wrapping add");
        check(cpu.base[0]  == 0,          "$zero: register 0 is hardwired to 0");
    }

    printf("\n%s (%d failure%s)\n", failed ? "FAILED" : "PASSED", failed, failed == 1 ? "" : "s");
    return failed ? 1 : 0;
}
