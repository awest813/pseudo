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

    // Try to load a PS1 game image (BIN) from the romdisk then the GD-ROM.
    const char *game_candidates[] = {
        "/rd/PSX.BIN",
        "/rd/psx.bin",
        "/rd/GAME.BIN",
        "/rd/game.bin",
        "/cd/PSX.BIN",
        "/cd/psx.bin",
        "/cd/GAME.BIN",
        "/cd/game.bin",
    };

    bool game_loaded = false;
    for (const char *path : game_candidates) {
        FILE *fp = fopen(path, "rb");
        if (fp) {
            fclose(fp);
            printf("Loading game: %s\n", path);
            psx.iso(path);
            game_loaded = true;
            break;
        }
    }

    if (!game_loaded) {
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
