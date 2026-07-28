#include "Global.h"


static const int xaPos[5] = { 0, 60, 115, 98, 122 };
static const int xaNeg[5] = { 0, 0, -52, -55, -60 };

static void decode28(const ub *group, int blk, int nibble, sh *dst,
                     sh *old, sh *older) {
    int shift = 12 - (group[4 + blk * 2 + nibble] & 0x0f);
    if (shift < 0) {
        shift = 0;
    }

    int filter = (group[4 + blk * 2 + nibble] >> 4) & 0x0f;
    if (filter > 4) {
        filter = 4;
    }

    const int f0 = xaPos[filter];
    const int f1 = xaNeg[filter];

    for (int j = 0; j < 28; j++) {
        int t = (group[16 + blk + j * 4] >> (nibble * 4)) & 0x0f;
        if (t & 8) {
            t -= 16;
        }

        int s = (t << shift) + (((int)*old * f0 + (int)*older * f1 + 32) / 64);
        if (s < SHRT_MIN) {
            s = SHRT_MIN;
        }
        if (s > SHRT_MAX) {
            s = SHRT_MAX;
        }

        dst[j] = (sh)s;
        *older = *old;
        *old = (sh)s;
    }
}

void xaReset(XADecodeState *state) {
    state->old[0] = state->old[1] = 0;
    state->older[0] = state->older[1] = 0;
}

int xaDecodeSector(const ub *sector, ub file, ub channel, XADecodeState *state,
                   sh *outL, sh *outR, int maxSamples) {
    const ub *sub = sector + 4;
    if (!(sub[2] & 0x80)) {
        return 0;
    }
    if (sub[0] != file || sub[1] != channel) {
        return 0;
    }

    const bool stereo = (sub[3] & 1) != 0;
    const ub *src = sector + 12;
    int out = 0;
    sh block[28];

    for (int group = 0; group < 18 && out < maxSamples; group++) {
        const ub *g = src + group * 128;

        for (int blk = 0; blk < 4 && out < maxSamples; blk++) {
            if (stereo) {
                decode28(g, blk, 0, block, &state->old[0], &state->older[0]);
                for (int i = 0; i < 28 && (out + i) < maxSamples; i++) {
                    outL[out + i] = block[i];
                }

                decode28(g, blk, 1, block, &state->old[1], &state->older[1]);
                for (int i = 0; i < 28 && (out + i) < maxSamples; i++) {
                    outR[out + i] = block[i];
                }

                out += 28;
            }
            else {
                decode28(g, blk, 0, block, &state->old[0], &state->older[0]);
                for (int i = 0; i < 28 && out < maxSamples; i++) {
                    outL[out] = block[i];
                    outR[out] = block[i];
                    out++;
                }

                decode28(g, blk, 1, block, &state->old[0], &state->older[0]);
                for (int i = 0; i < 28 && out < maxSamples; i++) {
                    outL[out] = block[i];
                    outR[out] = block[i];
                    out++;
                }
            }
        }
    }

    return out;
}
