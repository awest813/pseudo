class CstrMemcard {
    enum {
        MC_SIZE    = 128 * 1024,
        MC_SECTOR  = 128,
        MC_SECTORS = 1024,
    };

    enum Phase {
        PHASE_IDLE,
        PHASE_CMD,
        PHASE_READ_ID1,
        PHASE_READ_ID2,
        PHASE_READ_ADDR_MSB,
        PHASE_READ_ADDR_LSB,
        PHASE_READ_ACK1,
        PHASE_READ_ACK2,
        PHASE_READ_CONFIRM_MSB,
        PHASE_READ_CONFIRM_LSB,
        PHASE_READ_DATA,
        PHASE_READ_CHECKSUM,
        PHASE_READ_END,
        PHASE_WRITE_ID1,
        PHASE_WRITE_ID2,
        PHASE_WRITE_ADDR_MSB,
        PHASE_WRITE_ADDR_LSB,
        PHASE_WRITE_DATA,
        PHASE_WRITE_CHECKSUM,
        PHASE_WRITE_ACK1,
        PHASE_WRITE_ACK2,
        PHASE_WRITE_END,
        PHASE_GET_ID,
        PHASE_INVALID,
    };

    ub data[MC_SIZE];
    ub flag;

    Phase phase;
    ub cmd;
    uw sector;
    ub addrMsb;
    ub addrLsb;
    ub checksum;
    int dataIdx;
    bool writeOk;

    ub rx[256];
    int rxCount;
    int rxPos;

    void enqueue(ub byte);
    uw sectorOffset() const;
    void finishWrite(bool ok);
    void loadFile();
    void saveFile() const;
    static const char *filePath();

public:
    void reset();
    void begin();
    void hostByte(ub byte);
    bool ready() const;
    ub read();
};

extern CstrMemcard memcard;
