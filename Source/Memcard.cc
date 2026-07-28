#include "Global.h"


CstrMemcard memcard;

const char *CstrMemcard::filePath() {
#if (defined(_arch_dreamcast) || defined(__DREAMCAST__)) && defined(__KOS__)
    return "/pc/memcard1.mcr";
#else
    return "memcard1.mcr";
#endif
}

void CstrMemcard::enqueue(ub byte) {
    if (rxCount < (int)sizeof(rx)) {
        rx[rxCount++] = byte;
    }
}

uw CstrMemcard::sectorOffset() const {
    const uw idx = sector & 0x3ff;
    return idx * MC_SECTOR;
}

void CstrMemcard::loadFile() {
    memset(data, 0xff, sizeof(data));

    FILE *fp = fopen(filePath(), "rb");
    if (!fp) {
        return;
    }

    fread(data, 1, sizeof(data), fp);
    fclose(fp);
}

void CstrMemcard::saveFile() const {
    FILE *fp = fopen(filePath(), "wb");
    if (!fp) {
        return;
    }

    fwrite(data, 1, sizeof(data), fp);
    fclose(fp);
}

void CstrMemcard::reset() {
    phase   = PHASE_IDLE;
    cmd     = 0;
    sector  = 0;
    flag    = 0x08;
    rxCount = 0;
    rxPos   = 0;
    loadFile();
}

void CstrMemcard::begin() {
    phase   = PHASE_CMD;
    cmd     = 0;
    rxCount = 0;
    rxPos   = 0;
}

void CstrMemcard::finishWrite(bool ok) {
    flag = (ub)((flag & ~0x04) | (ok ? 0 : 0x04));
    if (ok) {
        saveFile();
    }
}

void CstrMemcard::hostByte(ub byte) {
    switch (phase) {
        case PHASE_CMD:
            cmd = byte;
            enqueue(flag);
            switch (byte) {
                case 0x52:
                    phase = PHASE_READ_ID1;
                    checksum = 0;
                    dataIdx = 0;
                    return;
                case 0x57:
                    phase = PHASE_WRITE_ID1;
                    checksum = 0;
                    dataIdx = 0;
                    writeOk = true;
                    return;
                case 0x53:
                    phase = PHASE_GET_ID;
                    dataIdx = 0;
                    return;
                default:
                    phase = PHASE_INVALID;
                    return;
            }

        case PHASE_READ_ID1:
            enqueue(0x5a);
            phase = PHASE_READ_ID2;
            return;

        case PHASE_READ_ID2:
            enqueue(0x5d);
            phase = PHASE_READ_ADDR_MSB;
            return;

        case PHASE_READ_ADDR_MSB:
            addrMsb = byte;
            enqueue(0x00);
            phase = PHASE_READ_ADDR_LSB;
            return;

        case PHASE_READ_ADDR_LSB:
            addrLsb = byte;
            sector = (uw)addrMsb | ((uw)addrLsb << 8);
            checksum = (ub)(addrMsb ^ addrLsb);
            enqueue(addrMsb);
            phase = PHASE_READ_ACK1;
            return;

        case PHASE_READ_ACK1:
            enqueue(0x5c);
            phase = PHASE_READ_ACK2;
            return;

        case PHASE_READ_ACK2:
            enqueue(0x5d);
            phase = PHASE_READ_CONFIRM_MSB;
            return;

        case PHASE_READ_CONFIRM_MSB:
            enqueue(addrMsb);
            phase = PHASE_READ_CONFIRM_LSB;
            return;

        case PHASE_READ_CONFIRM_LSB:
            enqueue(addrLsb);
            phase = PHASE_READ_DATA;
            dataIdx = 0;
            return;

        case PHASE_READ_DATA:
            {
                const uw off = sectorOffset() + (uw)dataIdx;
                const ub value = data[off % MC_SIZE];
                checksum ^= value;
                enqueue(value);
                if (++dataIdx >= MC_SECTOR) {
                    phase = PHASE_READ_CHECKSUM;
                }
            }
            return;

        case PHASE_READ_CHECKSUM:
            enqueue(checksum);
            phase = PHASE_READ_END;
            return;

        case PHASE_READ_END:
            enqueue(0x47);
            phase = PHASE_IDLE;
            return;

        case PHASE_WRITE_ID1:
            enqueue(0x5a);
            phase = PHASE_WRITE_ID2;
            return;

        case PHASE_WRITE_ID2:
            enqueue(0x5d);
            phase = PHASE_WRITE_ADDR_MSB;
            return;

        case PHASE_WRITE_ADDR_MSB:
            addrMsb = byte;
            enqueue(0x00);
            phase = PHASE_WRITE_ADDR_LSB;
            return;

        case PHASE_WRITE_ADDR_LSB:
            addrLsb = byte;
            sector = (uw)addrMsb | ((uw)addrLsb << 8);
            checksum = (ub)(addrMsb ^ addrLsb);
            enqueue(addrMsb);
            phase = PHASE_WRITE_DATA;
            dataIdx = 0;
            return;

        case PHASE_WRITE_DATA:
            {
                const uw off = sectorOffset() + (uw)dataIdx;
                enqueue(byte);
                data[off % MC_SIZE] = byte;
                checksum ^= byte;
                if (++dataIdx >= MC_SECTOR) {
                    phase = PHASE_WRITE_CHECKSUM;
                }
            }
            return;

        case PHASE_WRITE_CHECKSUM:
            writeOk = (byte == checksum);
            enqueue(byte);
            phase = PHASE_WRITE_ACK1;
            return;

        case PHASE_WRITE_ACK1:
            enqueue(0x5c);
            phase = PHASE_WRITE_ACK2;
            return;

        case PHASE_WRITE_ACK2:
            enqueue(0x5d);
            phase = PHASE_WRITE_END;
            return;

        case PHASE_WRITE_END:
            if (writeOk) {
                finishWrite(true);
                enqueue(0x47);
            }
            else {
                finishWrite(false);
                enqueue(0x4e);
            }
            phase = PHASE_IDLE;
            return;

        case PHASE_GET_ID:
            switch (dataIdx++) {
                case 0: enqueue(0x5a); return;
                case 1: enqueue(0x5d); return;
                case 2: enqueue(0x5c); return;
                case 3: enqueue(0x5d); return;
                case 4: enqueue(0x04); return;
                case 5: enqueue(0x00); return;
                case 6: enqueue(0x00); return;
                case 7:
                    enqueue(0x80);
                    phase = PHASE_IDLE;
                    return;
                default:
                    phase = PHASE_IDLE;
                    return;
            }

        case PHASE_INVALID:
            enqueue(0xff);
            phase = PHASE_IDLE;
            return;

        default:
            phase = PHASE_IDLE;
            return;
    }
}

bool CstrMemcard::ready() const {
    return rxPos < rxCount;
}

ub CstrMemcard::read() {
    if (rxPos >= rxCount) {
        return 0xff;
    }

    return rx[rxPos++];
}
