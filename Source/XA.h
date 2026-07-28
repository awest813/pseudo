// CD-ROM XA ADPCM sector decoder (Mode 2 Form 2 audio).

struct XADecodeState {
    sh old[2];
    sh older[2];
};

void xaReset(XADecodeState *state);
int xaDecodeSector(const ub *sector, ub file, ub channel, XADecodeState *state,
                   sh *outL, sh *outR, int maxSamples);
