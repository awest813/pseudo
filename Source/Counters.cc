#include "Global.h"


#define RTC_PORT \
    (addr >> 4) & 3


CstrCounters rootc;

void CstrCounters::reset() {
    for (auto &tmr : timer) {
        tmr.mode.data = 0;
        tmr.mode.irqRequest = 1; // idle = no IRQ pending
        tmr.current   = 0;
        tmr.bounds    = 0xffff;
        tmr.dest      = 0;
        tmr.temp      = 0;
        tmr.irqFired  = false;
    }
}

void CstrCounters::fireIrq(int port) {
    auto tmr = &timer[port];

    // One-shot: suppress further IRQs until the mode register is rewritten
    if (!tmr->mode.irqRepeat && tmr->irqFired) {
        return;
    }

    if (tmr->mode.irqToggle) {
        // Invert bit10; bus IRQ only when it transitions to 0 (active)
        tmr->mode.irqRequest ^= 1;
        if (tmr->mode.irqRequest != 0) {
            return;
        }
        bus.interruptSet(CstrBus::INT_RTC0 + port);
        if (!tmr->mode.irqRepeat) {
            tmr->irqFired = true;
            // Remains zero after the one-shot IRQ
        }
        return;
    }

    // Pulse: briefly assert (bit10=0), then return to idle (bit10=1)
    tmr->mode.irqRequest = 0;
    bus.interruptSet(CstrBus::INT_RTC0 + port);
    tmr->mode.irqRequest = 1;

    if (!tmr->mode.irqRepeat) {
        tmr->irqFired = true;
    }
}

void CstrCounters::update(uw frames) {
    for (int p = 0; p < 3; p++) {
        auto tmr = &timer[p];

        uw tval   = tmr->current;
        uw source = tmr->mode.clockSource >> ((p == 2) ? 1 : 0);

        float rate = 1.5f;
        if ((source & 1) == 1) {
            rate = table[p];
        }

        tval += (tmr->temp += frames) / rate;
        tmr->temp %= (uw)rate;

        if (tval >= tmr->dest) {
            if (tmr->mode.resetZero == ResetToZero::onDest) {
                tval = 0;
            }
            tmr->mode.reachedDest = 1;
            if (tmr->mode.irqWhenDest) {
                fireIrq(p);
            }
        }

        if (tval >= tmr->bounds) {
            if (tmr->mode.resetZero == ResetToZero::onBounds) {
                tval = 0;
            }
            tmr->mode.reachedBounds = 1;
            if (tmr->mode.irqWhenBounds) {
                fireIrq(p);
            }
        }

        tmr->current = (uh)tval;
    }
}

void CstrCounters::write(uw addr, uh data) {
    auto tmr = &timer[RTC_PORT];

    switch(addr & 0xf) {
        case 0:
            tmr->current = data;
            return;

        case 4:
            tmr->mode.data = data;
            tmr->mode.irqRequest = 1; // set after writing
            tmr->current  = 0;
            tmr->irqFired = false;
            return;

        case 8:
            tmr->dest = data;
            return;
    }

    printx("/// PSeudo RTC Write: %d <- 0x%x", (addr & 0xf), data);
}

uh CstrCounters::read(uw addr) {
    auto tmr = &timer[RTC_PORT];

    switch(addr & 0xf) {
        case 0:
            return tmr->current;

        case 4: {
            uh data = tmr->mode.data;
            tmr->mode.reachedDest   = 0;
            tmr->mode.reachedBounds = 0;
            return data;
        }

        case 8:
            return tmr->dest;
    }

    printx("/// PSeudo RTC Read: %d", (addr & 0xf));
    return 0;
}
