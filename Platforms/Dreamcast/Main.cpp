#include "Global.h"

#ifdef __KOS__
#include "../../Source/Global.h"

#include <dirent.h>

// KOS init flags: default hardware initialization
KOS_INIT_FLAGS(INIT_DEFAULT);

// Thread entry points
static void *cpu_thread(void *arg) {
    (void)arg;
    cpu.run();
    return NULL;
}

static void *audio_thread(void *arg) {
    (void)arg;
    audio.decodeStream();
    return NULL;
}

// Poll the Dreamcast controller and forward button changes to the SIO layer.
// cont_state_t.buttons is active-high: the maple driver normalizes the
// active-low wire format, so a set bit means the button is pressed.
// The upper bits of *prev store the previous digital state of the analog
// triggers: bit 16 = left trigger, bit 17 = right trigger.
static void poll_controller(uw *prev) {
    maple_device_t *dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (!dev) {
        return;
    }

    cont_state_t *state = (cont_state_t *)maple_dev_status(dev);
    if (!state) {
        return;
    }

    // Digital buttons
    uint16_t cur_buttons = (uint16_t)state->buttons;
    uint16_t changed = (uint16_t)(*prev & 0xffff) ^ cur_buttons;
    for (int i = 0; i < 16; i++) {
        uint16_t bit = (uint16_t)(1 << i);
        if (changed & bit) {
            sio.padListener((int)bit, (cur_buttons & bit) != 0);
        }
    }

    // Analog L/R triggers: treat as digital buttons above a threshold.
    bool ltrig_now = state->ltrig > DC_LTRIG_THRESHOLD;
    bool rtrig_now = state->rtrig > DC_RTRIG_THRESHOLD;
    bool ltrig_was = (*prev & DC_BTN_LTRIG) != 0;
    bool rtrig_was = (*prev & DC_BTN_RTRIG) != 0;

    if (ltrig_now != ltrig_was) {
        sio.padListener(DC_BTN_LTRIG, ltrig_now);
    }
    if (rtrig_now != rtrig_was) {
        sio.padListener(DC_BTN_RTRIG, rtrig_now);
    }

    *prev = (uw)cur_buttons
          | (ltrig_now ? (uw)DC_BTN_LTRIG : 0u)
          | (rtrig_now ? (uw)DC_BTN_RTRIG : 0u);
}

// --- Media discovery -------------------------------------------------------
// Files on the romdisk and GD-ROM are classified by content, not by name,
// so BIOS dumps, disc images and PS-X EXE homebrew can be called anything.

enum MediaKind {
    MEDIA_NONE,
    MEDIA_BIOS, // 512KB raw BIOS dump
    MEDIA_EXE,  // "PS-X EXE" header
    MEDIA_DISC, // raw disc image, 2352 bytes per sector
    MEDIA_CUE,  // cue sheet naming a disc image
};

#define MEDIA_PATH_MAX 256

// At least this many sectors before a file can be a disc image, to avoid
// mistaking small binaries whose size happens to be sector-aligned
#define MEDIA_MIN_SECTORS 16

static bool extMatch(const char *name, const char *ext) {
    size_t n = strlen(name), e = strlen(ext);
    if (n < e + 1 || name[n - e - 1] != '.') {
        return false;
    }
    for (size_t i = 0; i < e; i++) {
        if (tolower((unsigned char)name[n - e + i]) != ext[i]) {
            return false;
        }
    }
    return true;
}

static MediaKind classify(const char *path) {
    if (extMatch(path, "cue")) {
        return MEDIA_CUE;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return MEDIA_NONE;
    }

    char magic[8] = { 0 };
    size_t got = fread(magic, 1, sizeof(magic), fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    if (got == sizeof(magic) && !memcmp(magic, "PS-X EXE", 8)) {
        return MEDIA_EXE;
    }
    if (size == 0x80000) {
        return MEDIA_BIOS;
    }
    if (size >= MEDIA_MIN_SECTORS * 2352 && size % 2352 == 0) {
        return MEDIA_DISC;
    }
    return MEDIA_NONE;
}

// Extract the first FILE "..." entry of a cue sheet, resolved relative
// to the sheet's own directory
static bool cueDataFile(const char *cuePath, char *out, size_t outSize) {
    FILE *fp = fopen(cuePath, "r");
    if (!fp) {
        return false;
    }

    bool found = false;
    char line[MEDIA_PATH_MAX];
    while (fgets(line, sizeof(line), fp)) {
        const char *f = strstr(line, "FILE");
        const char *q1 = f  ? strchr(f, '"')       : NULL;
        const char *q2 = q1 ? strchr(q1 + 1, '"')  : NULL;
        if (!q2) {
            continue;
        }

        const char *slash = strrchr(cuePath, '/');
        size_t dirLen  = slash ? (size_t)(slash - cuePath + 1) : 0;
        size_t nameLen = (size_t)(q2 - q1 - 1);
        if (dirLen + nameLen + 1 > outSize) {
            break;
        }
        memcpy(out, cuePath, dirLen);
        memcpy(out + dirLen, q1 + 1, nameLen);
        out[dirLen + nameLen] = 0;
        found = true;
        break;
    }
    fclose(fp);
    return found;
}

// Scan one directory, keeping the first hit of each kind. Earlier calls
// win: paths already filled in are not replaced.
static void scanMedia(const char *dir, char *bios, char *game, char *exe) {
    DIR *d = opendir(dir);
    if (!d) {
        return;
    }

    char bin[MEDIA_PATH_MAX] = "";
    char cue[MEDIA_PATH_MAX] = "";

    struct dirent *entry;
    while ((entry = readdir(d))) {
        char path[MEDIA_PATH_MAX];
        if (snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name) >= (int)sizeof(path)) {
            continue;
        }

        switch (classify(path)) {
            case MEDIA_BIOS:
                if (!bios[0]) {
                    strcpy(bios, path);
                }
                break;

            case MEDIA_EXE:
                if (!exe[0]) {
                    strcpy(exe, path);
                }
                break;

            case MEDIA_DISC:
                if (!bin[0]) {
                    strcpy(bin, path);
                }
                break;

            case MEDIA_CUE:
                if (!cue[0]) {
                    char ref[MEDIA_PATH_MAX];
                    if (cueDataFile(path, ref, sizeof(ref)) && classify(ref) == MEDIA_DISC) {
                        strcpy(cue, ref);
                    }
                }
                break;

            default:
                break;
        }
    }
    closedir(d);

    // The cue sheet names the true data file; prefer it over a bare image
    if (!game[0]) {
        strcpy(game, cue[0] ? cue : bin);
    }
}
#endif // __KOS__

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

#ifdef __KOS__
    // Initialize OpenGL via GLdc (KOS PowerVR OpenGL wrapper)
    glKosInit();

    // Set up the emulator display at the Dreamcast's native 640x480
    draw.init(DC_SCREEN_W, DC_SCREEN_H, 1);

    // Look for a BIOS, a disc image and homebrew on the romdisk first,
    // then the GD-ROM. Extra roots can be passed as arguments (dc-load).
    static char biosPath[MEDIA_PATH_MAX] = "";
    static char gamePath[MEDIA_PATH_MAX] = "";
    static char  exePath[MEDIA_PATH_MAX] = "";

    for (int i = 1; i < argc; i++) {
        scanMedia(argv[i], biosPath, gamePath, exePath);
    }
    scanMedia("/rd", biosPath, gamePath, exePath);
    scanMedia("/cd", biosPath, gamePath, exePath);

    if (!biosPath[0]) {
        printf("PSeudo: BIOS not found. Place a 512KB BIOS dump (e.g. SCPH1001.BIN)\n");
        printf("        on the romdisk or GD-ROM.\n");
        return 1;
    }

    printf("Loading BIOS: %s\n", biosPath);
    psx.init(biosPath);

    if (gamePath[0]) {
        printf("Loading game: %s\n", gamePath);
        psx.iso(gamePath);
    }
    else if (exePath[0]) {
        printf("Loading executable: %s\n", exePath);
        psx.executable(exePath);
    }
    else {
        printf("PSeudo: No game image found. Running BIOS shell.\n");
    }

    // Launch the CPU and audio threads
    kthread_t *t_cpu   = thd_create(0, cpu_thread,   NULL);
    kthread_t *t_audio = thd_create(0, audio_thread, NULL);

    // Main loop: poll controller and yield to other threads.
    // Upper bits of prev_buttons hold the previous digital trigger state.
    uw prev_buttons = 0; // Active-high: no bits set = no buttons pressed
    while (!psx.suspended) {
        poll_controller(&prev_buttons);
        thd_pass();
    }

    thd_join(t_cpu,   NULL);
    thd_join(t_audio, NULL);
#else
    printf("PSeudo Dreamcast port (host build)\n");
    printf("Build with the KOS toolchain targeting Dreamcast hardware.\n");
#endif // __KOS__

    return 0;
}
