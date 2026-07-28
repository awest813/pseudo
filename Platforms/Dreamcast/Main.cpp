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
// The word tracked in *prev holds the 16 CONT_* bits plus the synthetic
// DC_BTN_* bits for the analog triggers and the Select combo.
static void poll_controller(uw *prev) {
    maple_device_t *dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (!dev) {
        return;
    }

    cont_state_t *state = (cont_state_t *)maple_dev_status(dev);
    if (!state) {
        return;
    }

    uw cur = (uw)(state->buttons & 0xffff);

    // The analog stick doubles as the digital pad
    if (state->joyx < -DC_STICK_THRESHOLD) cur |= CONT_DPAD_LEFT;
    if (state->joyx >  DC_STICK_THRESHOLD) cur |= CONT_DPAD_RIGHT;
    if (state->joyy < -DC_STICK_THRESHOLD) cur |= CONT_DPAD_UP;
    if (state->joyy >  DC_STICK_THRESHOLD) cur |= CONT_DPAD_DOWN;

    // Analog L/R triggers as digital L1/R1
    bool ltrig = state->ltrig > DC_LTRIG_THRESHOLD;
    bool rtrig = state->rtrig > DC_RTRIG_THRESHOLD;
    if (ltrig) cur |= DC_BTN_LTRIG;
    if (rtrig) cur |= DC_BTN_RTRIG;

    // Dreamcast pads have no Select: both triggers + Start sends Select
    // (Start itself is suppressed while the combo is held)
    if (ltrig && rtrig && (cur & CONT_START)) {
        cur = (cur & ~(uw)CONT_START) | DC_BTN_SELECT;
    }

    // A+B+X+Y+Start: the Dreamcast convention to leave a game
    const uw quit = CONT_A | CONT_B | CONT_X | CONT_Y | CONT_START;
    if ((cur & quit) == quit) {
        psx.suspended = true;
    }

    uw changed = *prev ^ cur;
    for (int i = 0; i < DC_BTN_BITS; i++) {
        uw bit = 1u << i;
        if (changed & bit) {
            sio.padListener((int)bit, (cur & bit) != 0);
        }
    }
    *prev = cur;
}

// --- Media discovery -------------------------------------------------------
// Files on the romdisk and GD-ROM are classified by content, not by name,
// so BIOS dumps, disc images and PS-X EXE homebrew can be called anything.
// The entry types live in Menu.h, shared with the boot picker.

#include "Menu.h"

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

    if (got >= 4 && !memcmp(magic, "ECM\0", 4)) {
        return MEDIA_ECM;
    }
    if (got == sizeof(magic) && !memcmp(magic, "PS-X EXE", 8)) {
        return MEDIA_EXE;
    }
    if (size == 0x80000) {
        return MEDIA_BIOS;
    }
    if (size >= MEDIA_MIN_SECTORS * 2352 && size % 2352 == 0) {
        return MEDIA_DISC;
    }
    if (size >= MEDIA_MIN_SECTORS * 2048 && size % 2048 == 0) {
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

// Append a media entry, deduplicating by path (a cue sheet and the image
// it references resolve to the same file)
static int addEntry(MediaEntry *list, int count, const char *path, MediaKind kind) {
    if (count >= MEDIA_MAX_ENTRIES) {
        return count;
    }
    for (int i = 0; i < count; i++) {
        if (!strcmp(list[i].path, path)) {
            return count;
        }
    }
    strcpy(list[count].path, path);
    list[count].kind = kind;
    return count + 1;
}

// Scan one directory: the first BIOS found wins, every disc image and
// PS-X EXE is collected for the boot picker
static int scanMedia(const char *dir, char *bios, MediaEntry *games, int count) {
    DIR *d = opendir(dir);
    if (!d) {
        return count;
    }

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
                count = addEntry(games, count, path, MEDIA_EXE);
                break;

            case MEDIA_DISC:
                count = addEntry(games, count, path, MEDIA_DISC);
                break;

            case MEDIA_CUE:
                {
                    char ref[MEDIA_PATH_MAX];
                    if (cueDataFile(path, ref, sizeof(ref)) && classify(ref) == MEDIA_DISC) {
                        count = addEntry(games, count, ref, MEDIA_DISC);
                    }
                }
                break;

            case MEDIA_ECM:
                printf("PSeudo: Skipping ECM image (decompress to .bin/.img first): %s\n", path);
                break;

            default:
                break;
        }
    }
    closedir(d);

    return count;
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

    // Look for a BIOS, disc images and homebrew on the romdisk first,
    // then the GD-ROM. Extra roots can be passed as arguments (dc-load).
    static char biosPath[MEDIA_PATH_MAX] = "";
    static MediaEntry games[MEDIA_MAX_ENTRIES];
    int gameCount = 0;

    for (int i = 1; i < argc; i++) {
        gameCount = scanMedia(argv[i], biosPath, games, gameCount);
    }
    gameCount = scanMedia("/rd", biosPath, games, gameCount);
    gameCount = scanMedia("/cd", biosPath, games, gameCount);

    if (!biosPath[0]) {
        printf("PSeudo: BIOS not found. Place a 512KB BIOS dump (e.g. SCPH1001.BIN)\n");
        printf("        on the romdisk or GD-ROM.\n");
        return 1;
    }

    // Always offer the picker so a single game can still yield to the BIOS
    // shell, and so zero-game boots still confirm "Start BIOS".
    int pick = menuPickGame(games, gameCount);

    printf("Loading BIOS: %s\n", biosPath);
    psx.init(biosPath);

    if (pick >= 0) {
        if (games[pick].kind == MEDIA_DISC) {
            printf("Loading game: %s\n", games[pick].path);
            psx.iso(games[pick].path);
        }
        else {
            printf("Loading executable: %s\n", games[pick].path);
            psx.executable(games[pick].path);
        }
    }
    else {
        printf("PSeudo: No game selected. Running BIOS shell.\n");
    }

    // Launch the CPU and audio threads
    kthread_t *t_cpu   = thd_create(0, cpu_thread,   NULL);
    kthread_t *t_audio = thd_create(0, audio_thread, NULL);

    // Main loop: poll the controller at a fixed ~100Hz rate rather than
    // spinning on thd_pass() every reschedule. That would otherwise hit
    // the maple bus and fight the CPU/audio threads for timeslices far
    // more often than input actually needs to be sampled.
    // Upper bits of prev_buttons hold the previous digital trigger state.
    uw prev_buttons = 0; // Active-high: no bits set = no buttons pressed
    while (!psx.suspended) {
        poll_controller(&prev_buttons);
        thd_sleep(10);
    }

    thd_join(t_cpu,   NULL);
    thd_join(t_audio, NULL);
#else
    printf("PSeudo Dreamcast port (host build)\n");
    printf("Build with the KOS toolchain targeting Dreamcast hardware.\n");
#endif // __KOS__

    return 0;
}
