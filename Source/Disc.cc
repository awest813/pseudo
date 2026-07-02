#include "Global.h"


#define MSF2SECT(m, s, f) \
    (((m) * 60 + (s) - 2) * 75 + (f))


CstrDisc disc;

void CstrDisc::reset() {
    memset(&bfr, 0, UDF_DATASIZE);
    
    if (file) {
        fclose(file); file = 0;
    }
}

bool CstrDisc::open(const char *path) {
    file = fopen(path, "rb");
    
    if (!file) {
        return false;
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
        uw sectors = (uw)(ftell(file) / UDF_FRAMESIZERAW) + 150;

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
        memset(&bfr, 0, UDF_DATASIZE);
        return false;
    }
    
    fseek(file, MSF2SECT(BCD2INT(t[0]), BCD2INT(t[1]), BCD2INT(t[2])) * UDF_FRAMESIZERAW + 12, SEEK_SET);
    fread(bfr, 1, UDF_DATASIZE, file);
    
    return true;
}
