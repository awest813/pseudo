/* Rootcounter IRQ mode unit tests (once/repeat, pulse/toggle). */

#include "Global.h"


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

enum {
    MODE_IRQ_DEST   = 1 << 4,
    MODE_IRQ_BOUNDS = 1 << 5,
    MODE_REPEAT     = 1 << 6,
    MODE_TOGGLE     = 1 << 7,
    MODE_IRQ_REQ    = 1 << 10,
    MODE_REACHED_DEST = 1 << 11,
};

static void drainIrq() {
    // RTC interrupt dest=1 → need ~100 update ticks to latch I_STAT
    bus.update(100);
}

static bool rtc0Pending() {
    return (data16 & (1 << CstrBus::INT_RTC0)) != 0;
}

static void clearIStat() {
    data16 = 0;
    bus.reset();
}

int main() {
    mem.reset();
    rootc.reset();
    clearIStat();

  {
    // Mode write sets bit10 (irqRequest=1) and resets counter
    rootc.write(0x1f801100, 0x1234);
    rootc.write(0x1f801104, MODE_IRQ_DEST | MODE_REPEAT);
    check(rootc.read(0x1f801100) == 0, "mode write: counter reset");
    check((rootc.read(0x1f801104) & MODE_IRQ_REQ) != 0, "mode write: bit10 set");
  }

  {
    // Pulse + repeat: two target hits → two IRQs
    clearIStat();
    rootc.reset();
    rootc.write(0x1f801108, 10); // target
    rootc.write(0x1f801104, MODE_IRQ_DEST | MODE_REPEAT); // pulse, repeat, reset on FFFFh
    // Force counter near target via current write, then update past it
    rootc.write(0x1f801100, 9);
    // Need enough frames: rate is 1.5 with clockSource=0, so ~2 frames per tick
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "pulse+repeat: first IRQ");

    clearIStat();
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "pulse+repeat: second IRQ");
  }

  {
    // Pulse + once: only first IRQ fires
    clearIStat();
    rootc.reset();
    rootc.write(0x1f801108, 10);
    rootc.write(0x1f801104, MODE_IRQ_DEST); // pulse, one-shot
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "pulse+once: first IRQ");

    clearIStat();
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(!rtc0Pending(), "pulse+once: second IRQ suppressed");

    // Rewriting mode re-arms one-shot
    rootc.write(0x1f801104, MODE_IRQ_DEST);
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "pulse+once: rearmed after mode write");
  }

  {
    // Toggle + repeat: IRQ only when bit10 falls 1→0 (every other hit)
    clearIStat();
    rootc.reset();
    rootc.write(0x1f801108, 10);
    rootc.write(0x1f801104, MODE_IRQ_DEST | MODE_REPEAT | MODE_TOGGLE);
    check((rootc.read(0x1f801104) & MODE_IRQ_REQ) != 0, "toggle: bit10 starts set");

    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "toggle+repeat: first hit fires (1→0)");
    check((rootc.read(0x1f801104) & MODE_IRQ_REQ) == 0, "toggle: bit10 cleared");

    clearIStat();
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(!rtc0Pending(), "toggle+repeat: second hit silent (0→1)");
    check((rootc.read(0x1f801104) & MODE_IRQ_REQ) != 0, "toggle: bit10 set again");

    clearIStat();
    rootc.write(0x1f801100, 9);
    rootc.update(3);
    drainIrq();
    check(rtc0Pending(), "toggle+repeat: third hit fires (1→0)");
  }

  {
    // Reached-target flag set on hit, cleared on mode read
    clearIStat();
    rootc.reset();
    rootc.write(0x1f801108, 5);
    rootc.write(0x1f801104, MODE_IRQ_DEST | MODE_REPEAT);
    rootc.write(0x1f801100, 4);
    rootc.update(3);
    // Peek mode without going through read() clear — use a raw approach:
    // first read returns flags then clears
    const uh mode = rootc.read(0x1f801104);
    check((mode & MODE_REACHED_DEST) != 0, "reachedDest set on target");
    check((rootc.read(0x1f801104) & MODE_REACHED_DEST) == 0, "reachedDest cleared on read");
  }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
