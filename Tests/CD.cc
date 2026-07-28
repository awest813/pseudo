/* CD-ROM controller unit tests: ReadN/ReadS ACK and MODE1 GetlocL. */

#include "Global.h"

#include <cstdio>
#include <cstring>


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

static void pump(uw cycles = 1000) {
    for (uw i = 0; i < cycles; i++) {
        cd.update(1);
    }
}

static void ackIrq() {
    cd.write(0x1f801800, 1);
    cd.write(0x1f801803, 0x07);
    cd.write(0x1f801800, 0);
}

static ub readIrq() {
    cd.write(0x1f801800, 1);
    const ub irq = cd.read(0x1f801803) & 0x07;
    cd.write(0x1f801800, 0);
    return irq;
}

static void drainResult() {
    (void)cd.read(0x1f801801);
}

static void stopReads() {
    cd.write(0x1f801800, 0);
    cd.write(0x1f801801, 9); // Pause
    pump(2000);
    drainResult();
    ackIrq();
    pump(2000);
    drainResult();
    ackIrq();
}

int main() {
    {
        cd.reset();
        cd.write(0x1f801800, 0);
        cd.write(0x1f801801, 27);
        pump(2000);
        check(readIrq() == 3, "ReadS: ACK interrupt");
        drainResult();
        ackIrq();
        stopReads();
    }

    {
        cd.reset();
        cd.write(0x1f801800, 0);
        cd.write(0x1f801801, 6);
        pump(2000);
        check(readIrq() == 3, "ReadN: ACK interrupt");
        drainResult();
        ackIrq();
        stopReads();
    }

    {
        const char *path = "/tmp/pseudo-cd-2048.bin";
        FILE *fp = fopen(path, "wb");
        check(fp != nullptr, "write MODE1 image for GetlocL");
        if (fp) {
            ub sector[2048];
            for (int s = 0; s < 16; s++) {
                memset(sector, 0, sizeof(sector));
                sector[0] = (ub)(0xa0 + s);
                fwrite(sector, 1, 2048, fp);
            }
            fclose(fp);
        }

        disc.reset();
        check(disc.open(path), "open MODE1 for GetlocL");

        ub msf[3] = { 0x00, 0x02, 0x00 };
        check(disc.trackRead(msf), "trackRead MODE1");
        check(disc.bfr[0] == 0x00 && disc.bfr[1] == 0x02 && disc.bfr[3] == 0x02,
              "MODE1 synthesized header");
        check(disc.bfr[12] == 0xa0, "MODE1 user data at +12");

        cd.reset();
        cd.write(0x1f801800, 0);
        cd.write(0x1f801802, 0x10);
        cd.write(0x1f801801, 14); // Setmode
        pump(2000);
        drainResult();
        ackIrq();

        cd.write(0x1f801802, 0x00);
        cd.write(0x1f801802, 0x02);
        cd.write(0x1f801802, 0x00);
        cd.write(0x1f801801, 2); // Setloc
        pump(2000);
        drainResult();
        ackIrq();

        cd.write(0x1f801801, 6); // ReadN
        pump(2000);
        drainResult();
        ackIrq();

        pump(200000);
        check(readIrq() == 1, "ReadN: DATA_READY");
        drainResult();
        ackIrq();

        cd.write(0x1f801801, 16); // GetlocL
        pump(2000);
        check(readIrq() == 3, "GetlocL: ACK");
        ub loc[8];
        for (int i = 0; i < 8; i++) {
            loc[i] = cd.read(0x1f801801);
        }
        check(loc[0] == 0x00 && loc[1] == 0x02 && loc[2] == 0x00 && loc[3] == 0x02,
              "GetlocL: MM:SS:FF mode");

        cd.write(0x1f801800, 0);
        cd.write(0x1f801803, 0x80);
        check(cd.read(0x1f801802) == 0x00 && cd.read(0x1f801802) == 0x02 &&
              cd.read(0x1f801802) == 0x00 && cd.read(0x1f801802) == 0x02,
              "DATA fifo: MODE1 header");

        stopReads();
        disc.reset();
        remove(path);
    }

    {
        cd.reset();
        cd.write(0x1f801800, 0);
        cd.write(0x1f801801, 3); // Play
        pump(2000);
        drainResult();
        ackIrq();
        cd.write(0x1f801801, 8); // Stop
        pump(2000);
        const ub stat = cd.read(0x1f801801);
        check((stat & 0x80) == 0, "Stop: clears playing bit");
    }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
