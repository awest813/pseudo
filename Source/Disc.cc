#include "Global.h"


#define MSF2SECT(m, s, f) \
    (((m) * 60 + (s) - 2) * 75 + (f))


static bool hasCdSync(FILE *fp) {
    ub h[12];

    if (fread(h, 1, sizeof(h), fp) != sizeof(h)) {
        return false;
    }

    rewind(fp);
    return h[0] == 0 && h[1] == 0xff && h[2] == 0xff && h[3] == 0xff
        && h[4] == 0xff && h[5] == 0xff && h[6] == 0xff && h[7] == 0xff
        && h[8] == 0xff && h[9] == 0xff && h[10] == 0xff;
}


CstrDisc disc;

void CstrDisc::reset() {
    memset(&bfr, 0, UDF_DATASIZE);

    if (file) {
        fclose(file); file = 0;
    }

    sectorSize = UDF_FRAMESIZERAW;
}

bool CstrDisc::open(const char *path) {
    file = fopen(path, "rb");

    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    sectorSize = UDF_FRAMESIZERAW;
    if (size >= 16L * 2048 && (size % 2048) == 0) {
        if ((size % UDF_FRAMESIZERAW) != 0 || !hasCdSync(file)) {
            sectorSize = 2048;
        }
    }

    return true;
}

bool CstrDisc::exists() {
    if (!file) {
        return false;
    }

    return true;
}

void CstrDisc::fetchTN(ub *b) {
    b[0] = 1;
    b[1] = 1;
}

void CstrDisc::fetchTD(ub track, ub *b) {
    // b = { frame, second, minute }
    if (track == 0 && file) {
        // Lead-out: total disc length from the image size, plus the
        // 150-sector (2 second) pregap
        fseek(file, 0, SEEK_END);
        uw sectors = (uw)(ftell(file) / sectorSize) + 150;

        b[0] = sectors % 75;
        b[1] = (sectors / 75) % 60;
        b[2] = sectors / (75 * 60);
        return;
    }

    // Track start: the data track begins right after the pregap
    b[0] = 0;
    b[1] = 2;
    b[2] = 0;
}

bool CstrDisc::trackRead(ub *t) {
    if (!file) {
        memset(&bfr, 0, UDF_FRAMESIZERAW);
        return false;
    }

    const long sector = MSF2SECT(BCD2INT(t[0]), BCD2INT(t[1]), BCD2INT(t[2]));

    if (sectorSize == 2048) {
        // Present MODE1/2048 images in the same post-sync layout as raw
        // sectors: header at [0..3], user data at [12..] so CD mode 0x00
        // (skip 12) and GetlocL both work.
        fseek(file, sector * 2048, SEEK_SET);
        memset(bfr, 0, UDF_FRAMESIZERAW);
        bfr[0] = t[0];
        bfr[1] = t[1];
        bfr[2] = t[2];
        bfr[3] = 0x02; // MODE1
        fread(bfr + 12, 1, 2048, file);
    }
    else {
        fseek(file, sector * UDF_FRAMESIZERAW + 12, SEEK_SET);
        fread(bfr, 1, UDF_DATASIZE, file);
    }

    return true;
}
