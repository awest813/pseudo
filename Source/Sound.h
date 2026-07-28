class CstrAudio {
    enum {
        SPU_SAMPLE_RATE    = 44100,
        SPU_SAMPLE_SIZE    = 1024,
        SPU_SAMPLE_COUNT   = SPU_SAMPLE_SIZE / 4,
        SPU_MAX_CHAN       = 24 + 1,
#ifdef DREAMCAST
        // Fewer queued buffers on DC: each holds 1 KB of stereo samples
        SPU_ALC_BUF_AMOUNT = 8
#else
        SPU_ALC_BUF_AMOUNT = 16
#endif
        ,
        XA_SAMPLE_RATE     = 37800,
        XA_BUF_SAMPLES = 32768
    };
    
    const int f[5][2] = {
        { 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 }
    };
    
    uh spuMem[256 * 1024];
    uh sbuf[SPU_SAMPLE_SIZE];
    uw spuAddr;
    
    // OpenAL
    ALCdevice *device;
    ALuint source;
    ALuint bfr[SPU_ALC_BUF_AMOUNT];
    
    struct {
        bool isNew, active, repeat;
        sw spos, bpos, freq, sample;
        sh volumeL, volumeR;
        sw bfr[28], s[2];
        
        // Address
        sw saddr; // Start
        sw paddr; // Current
        sw raddr; // Return
    } spuVoices[SPU_MAX_CHAN];

    sh xaL[XA_BUF_SAMPLES];
    sh xaR[XA_BUF_SAMPLES];
    int xaRead, xaWrite, xaCount;
    int xaFrac;
    XADecodeState xaState;
    sh cdVolL, cdVolR;

    sh setVolume(sh);
    void voiceOn(uw);
    void voiceOff(uw);
    void freeBuffers();
    void mixXA(int samples);
    
public:
    CstrAudio() {
        // OpenAL
        device = alcOpenDevice(0);
        ALCcontext *ctx = alcCreateContext(device, 0);
        alcMakeContextCurrent(ctx);
        alGenSources(1, &source);
        alGenBuffers(SPU_ALC_BUF_AMOUNT, bfr);
        
        for (auto &item : bfr) {
            alBufferData(item, AL_FORMAT_STEREO16, sbuf, SPU_SAMPLE_SIZE, SPU_SAMPLE_RATE);
        }
        
        alSourceQueueBuffers(source, SPU_ALC_BUF_AMOUNT, bfr);
    }
    
    ~CstrAudio() {
        alDeleteSources(1, &source);
        alDeleteBuffers(SPU_ALC_BUF_AMOUNT, bfr);
        alcCloseDevice(device);
    }
    
    void reset();
    void decodeStream();
    void decodeXA(const ub *sector, ub file, ub channel);
    void write(uw, uh);
    uh read(uw);
    void executeDMA(CstrBus::castDMA *);
};

extern CstrAudio audio;
