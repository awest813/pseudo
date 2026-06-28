#include "Global.h"

#ifdef __KOS__
#include "../../Source/Global.h"

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
// KOS button bits are active-low (0 = pressed, 1 = released).
static void poll_controller(uint16_t *prev) {
    maple_device_t *dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (!dev) {
        return;
    }

    cont_state_t *state = (cont_state_t *)maple_dev_status(dev);
    if (!state) {
        return;
    }

    uint16_t changed = *prev ^ state->buttons;
    for (int i = 0; i < 16; i++) {
        uint16_t bit = (uint16_t)(1 << i);
        if (changed & bit) {
            // pushed = true when bit transitions 1->0 (button pressed)
            sio.padListener((int)bit, !(state->buttons & bit));
        }
    }
    *prev = state->buttons;
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

    // Try to load the PS1 BIOS from the romdisk first, then fall back to GD-ROM
    const char *bios_candidates[] = {
        "/rd/SCPH1001.BIN",
        "/rd/scph1001.bin",
        "/cd/SCPH1001.BIN",
        "/cd/scph1001.bin",
    };

    bool bios_loaded = false;
    for (const char *path : bios_candidates) {
        FILE *fp = fopen(path, "rb");
        if (fp) {
            fclose(fp);
            psx.init(path);
            bios_loaded = true;
            printf("Loaded BIOS: %s\n", path);
            break;
        }
    }

    if (!bios_loaded) {
        printf("PSeudo: BIOS not found. Place SCPH1001.BIN on romdisk or GD-ROM.\n");
        return 1;
    }

    // Launch the CPU and audio threads
    kthread_t *t_cpu   = thd_create(0, cpu_thread,   NULL);
    kthread_t *t_audio = thd_create(0, audio_thread, NULL);

    // Main loop: poll controller and yield to other threads
    uint16_t prev_buttons = 0xffff; // All bits high = no buttons pressed
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
