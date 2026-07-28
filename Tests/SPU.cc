/* SPU Key On / Key Off unit tests. */

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

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
