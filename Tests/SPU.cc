/* SPU Key On / Key Off / ENDX unit tests. */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

int main() {
    audio.reset();

    // Point voice 0 at a non-zero SPU address so Key On arms isNew
    audio.write(0x1f801c06, 0x0200); // sound address << 3 internally
    audio.write(0x1f801d88, 0x0001); // Key On voice 0

    // Channel +0xC hack reports isNew
    check(audio.read(0x1f801c0c) == 1, "key on: voice 0 armed");

    audio.write(0x1f801d8c, 0x0001); // Key Off voice 0
    check(audio.read(0x1f801c0c) == 0, "key off: voice 0 disarmed");

    // Key On voice 16 via Sound On 2
    audio.write(0x1f801c06 + (16 * 0x10), 0x0400);
    audio.write(0x1f801d8a, 0x0001); // Key On bit 0 of high word → voice 16
    check(audio.read(0x1f801c0c + (16 * 0x10)) == 1, "key on: voice 16 armed");

    audio.write(0x1f801d8e, 0x0001); // Key Off voice 16
    check(audio.read(0x1f801c0c + (16 * 0x10)) == 0, "key off: voice 16 disarmed");

    // Voices without a start address stay quiet on Key On
    audio.write(0x1f801c16, 0); // voice 1 saddr = 0
    audio.write(0x1f801d88, 0x0002);
    check(audio.read(0x1f801c1c) == 0, "key on: zero saddr ignored");

    // --- ENDX: latches when an ADPCM block with the end bit is decoded ---
    audio.reset();

    // Load a single 16-byte ADPCM block at SPU addr 0x1000 with end flag
    audio.write(0x1f801da6, 0x0200); // transfer address = 0x1000
    audio.write(0x1f801da8, 0x0100); // shift/predict=0, flags=end
    for (int i = 0; i < 7; i++) {
        audio.write(0x1f801da8, 0);
    }

    audio.write(0x1f801c00, 0x3fff); // volume L
    audio.write(0x1f801c02, 0x3fff); // volume R
    audio.write(0x1f801c04, 0x1000); // pitch → 1 ADPCM sample per output
    audio.write(0x1f801c06, 0x0200); // start at 0x1000
    audio.write(0x1f801d88, 0x0001); // Key On voice 0

    check(audio.read(0x1f801d9c) == 0, "ENDX clear after key on");

    audio.step(); // consume the ending block

    check((audio.read(0x1f801d9c) & 0x0001) != 0, "ENDX set after ADPCM end");
    check(audio.read(0x1f801c0c) == 0, "voice inactive after end");

    // Key On clears the latched ENDX bit
    audio.write(0x1f801d88, 0x0001);
    check(audio.read(0x1f801d9c) == 0, "ENDX cleared on re-key");

    // Voice 16 ENDX lives in the high register
    audio.reset();
    audio.write(0x1f801da6, 0x0400); // 0x2000
    audio.write(0x1f801da8, 0x0100);
    for (int i = 0; i < 7; i++) {
        audio.write(0x1f801da8, 0);
    }
    audio.write(0x1f801c04 + (16 * 0x10), 0x1000);
    audio.write(0x1f801c06 + (16 * 0x10), 0x0400);
    audio.write(0x1f801d8a, 0x0001);
    audio.step();
    check((audio.read(0x1f801d9e) & 0x0001) != 0, "ENDX2 set for voice 16");

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
