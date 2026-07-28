class CstrDisc {
    FILE *file = nullptr;
    uw sectorSize = UDF_FRAMESIZERAW;

public:
    enum {
        UDF_FRAMESIZERAW = 2352,
        UDF_DATASIZE = UDF_FRAMESIZERAW - 12
    };

    ub bfr[UDF_FRAMESIZERAW];

    void reset();
    bool open(const char *);
    bool exists();
    void fetchTN(ub *);
    void fetchTD(ub, ub *);
    bool trackRead(ub *);
};

extern CstrDisc disc;
