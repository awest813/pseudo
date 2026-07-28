/* Disc I/O unit tests: MODE1/2048 and raw 2352 image handling. */

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

static bool writeImage(const char *path, uw sectorSize, int sectors, bool withSync) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return false;
    }

    for (int s = 0; s < sectors; s++) {
        ub sector[2352] = { 0 };
        if (withSync && s == 0) {
            sector[0] = 0x00;
            memset(sector + 1, 0xff, 10);
        }

        if (sectorSize == 2048) {
            sector[0] = (ub)(s & 0xff);
            fwrite(sector, 1, 2048, fp);
        }
        else {
            sector[12] = (ub)(s & 0xff);
            fwrite(sector, 1, 2352, fp);
        }
    }

    fclose(fp);
    return true;
}

int main() {
    const char *mode1 = "/tmp/pseudo-disc-2048.bin";
    const char *raw   = "/tmp/pseudo-disc-2352.bin";

    // --- MODE1 / 2048-byte sectors --------------------------------------
    {
        check(writeImage(mode1, 2048, 32, false), "write MODE1 image");

        disc.reset();
        check(disc.open(mode1), "open MODE1 image");

        ub td[3] = { 0 };
        disc.fetchTD(0, td);

        const uw sectors = 32 + 150;
        check(td[0] == sectors % 75,              "MODE1 fetchTD: frames");
        check(td[1] == (sectors / 75) % 60,       "MODE1 fetchTD: seconds");
        check(td[2] == sectors / (75 * 60),       "MODE1 fetchTD: minutes");

        ub msf[3] = { 0x00, 0x02, 0x00 };
        check(disc.trackRead(msf),                "MODE1 trackRead");
        check(disc.bfr[0] == 0,                   "MODE1 trackRead: sector 0 marker");
    }

    // --- Raw 2352-byte sectors with CD sync -----------------------------
    {
        check(writeImage(raw, 2352, 32, true), "write raw image");

        disc.reset();
        check(disc.open(raw), "open raw image");

        ub td[3] = { 0 };
        disc.fetchTD(0, td);

        const uw sectors = 32 + 150;
        check(td[0] == sectors % 75,              "raw fetchTD: frames");
        check(td[1] == (sectors / 75) % 60,       "raw fetchTD: seconds");

        ub msf[3] = { 0x00, 0x02, 0x00 };
        check(disc.trackRead(msf),                "raw trackRead");
        check(disc.bfr[0] == 0,                   "raw trackRead: sector 0 marker");
    }

    disc.reset();
    remove(mode1);
    remove(raw);

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
