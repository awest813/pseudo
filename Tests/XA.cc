/* XA ADPCM unit tests. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

int main() {
    ub sector[CstrDisc::UDF_DATASIZE] = { 0 };
    sector[4] = 1;  // file
    sector[5] = 1;  // channel
    sector[6] = 0x80;
    sector[7] = 0x00; // mono

    // First sound group, block 0: shift 0 (header nibble 12), filter 0, sample +1
    sector[12 + 4] = 0x0C;
    sector[12 + 16] = 0x01;

    XADecodeState state;
    xaReset(&state);

    sh left[4096];
    sh right[4096];
    const int count = xaDecodeSector(sector, 1, 1, &state, left, right, 4096);

    check(count == 4032, "mono sector sample count");
    check(left[0] == 1, "first decoded sample");
    check(right[0] == 1, "mono right matches left");

    check(xaDecodeSector(sector, 2, 1, &state, left, right, 4096) == 0,
          "wrong file filtered");

    sector[7] = 0x01; // stereo
    xaReset(&state);
    const int stereoCount = xaDecodeSector(sector, 1, 1, &state, left, right, 4096);
    check(stereoCount == 2016, "stereo sector sample count");
    check(left[0] == 1, "stereo left sample");
    check(right[0] == 0, "stereo right silent when only left nibble set");

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
