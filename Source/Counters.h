class CstrCounters {
    enum class ResetToZero: uh {
        onDest   = 1,
        onBounds = 0,
    };

    struct {
        // Mode breakdown — layout matches 1F801104h bit fields
        union {
            struct {
                uh syncEnable         : 1; // bit 0
                uh syncMode           : 2; // bits 1-2
                ResetToZero resetZero : 1; // bit 3
                uh irqWhenDest        : 1; // bit 4
                uh irqWhenBounds      : 1; // bit 5
                uh irqRepeat          : 1; // bit 6 (0=once, 1=repeat)
                uh irqToggle          : 1; // bit 7 (0=pulse, 1=toggle)
                uh clockSource        : 2; // bits 8-9
                uh irqRequest         : 1; // bit 10 (0=IRQ, 1=no IRQ)
                uh reachedDest        : 1; // bit 11
                uh reachedBounds      : 1; // bit 12
                uh                    : 3; // bits 13-15
            };

            uh data;
        } mode;

        uh current;
        uh bounds;
        uh dest;
        uw temp;
        bool irqFired; // one-shot latch until next mode write
    } timer[3];

    // Counters rate
    const uh table[3] = {
        6, 3413, (uh)(8 * 1.5f)
    };

    void fireIrq(int port);

public:
    void reset();
    void update(uw);
    void write(uw, uh);
    uh read(uw);
};

extern CstrCounters rootc;
