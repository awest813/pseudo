// Boot menu for the Dreamcast port: lists the media found on the
// romdisk / GD-ROM and lets the player pick with the controller.
// Text is drawn with a GL texture atlas built from the Dreamcast's
// BIOS font (bfont), so no font asset needs to ship with the binary.

#include "Global.h"

#ifdef __KOS__
#include "../../Source/Global.h"
#include "Menu.h"

#include <dc/biosfont.h>

// bfont glyphs are 12x24, 1bpp, 3 bytes per 2 rows
#define FONT_CW    12
#define FONT_CH    24
#define FONT_FIRST 32
#define FONT_LAST  126
#define FONT_COLS  16

// PVR textures must be power-of-two
#define ATLAS_W 256
#define ATLAS_H 256

#define MENU_VISIBLE  12
#define MENU_TIMEOUT (10 * 60) // frames without a controller before auto-boot

static GLuint fontTex = 0;

// Unpack one 1bpp bfont glyph into the RGBA atlas at (cellX, cellY).
// Every 3 bytes hold 2 rows of 12 bits, high bits first.
static void unpackChar(const unsigned char *src, unsigned int *atlas, int cellX, int cellY) {
    for (int y = 0; y < FONT_CH; y += 2) {
        const unsigned char *b = src + (y >> 1) * 3;
        unsigned int row0 = ((unsigned int)b[0] << 4) | (b[1] >> 4);
        unsigned int row1 = (((unsigned int)b[1] & 0x0f) << 8) | b[2];

        for (int x = 0; x < FONT_CW; x++) {
            unsigned int m = 1u << (FONT_CW - 1 - x);
            atlas[(cellY + y)     * ATLAS_W + cellX + x] = (row0 & m) ? 0xffffffffu : 0;
            atlas[(cellY + y + 1) * ATLAS_W + cellX + x] = (row1 & m) ? 0xffffffffu : 0;
        }
    }
}

static void menuInitFont(void) {
    if (fontTex) {
        return;
    }

    unsigned int *atlas = new unsigned int[ATLAS_W * ATLAS_H];
    memset(atlas, 0, ATLAS_W * ATLAS_H * sizeof(unsigned int));

    for (int ch = FONT_FIRST; ch <= FONT_LAST; ch++) {
        unsigned char *src = (unsigned char *)bfont_find_char(ch);
        if (!src) {
            continue;
        }
        int i = ch - FONT_FIRST;
        unpackChar(src, atlas, (i % FONT_COLS) * FONT_CW, (i / FONT_COLS) * FONT_CH);
    }

    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_W, ATLAS_H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, atlas);

    delete[] atlas;
}

static void drawText(int x, int y, const char *s) {
    glBegin(GL_QUADS);
    for (; *s; s++, x += FONT_CW) {
        unsigned char c = (unsigned char)*s;
        if (c <= FONT_FIRST || c > FONT_LAST) {
            continue; // space and anything unprintable advance silently
        }
        int i = c - FONT_FIRST;
        float u0 = ((i % FONT_COLS) * FONT_CW) / (float)ATLAS_W;
        float v0 = ((i / FONT_COLS) * FONT_CH) / (float)ATLAS_H;
        float u1 = u0 + FONT_CW / (float)ATLAS_W;
        float v1 = v0 + FONT_CH / (float)ATLAS_H;

        glTexCoord2f(u0, v0); glVertex2f((float)x,           (float)y);
        glTexCoord2f(u1, v0); glVertex2f((float)(x + FONT_CW), (float)y);
        glTexCoord2f(u1, v1); glVertex2f((float)(x + FONT_CW), (float)(y + FONT_CH));
        glTexCoord2f(u0, v1); glVertex2f((float)x,           (float)(y + FONT_CH));
    }
    glEnd();
}

static const char *baseName(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

// Current buttons, with the analog stick folded into the D-pad.
// Returns (uw)-1 when no controller is attached.
static uw menuButtons(void) {
    maple_device_t *dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (!dev) {
        return (uw)-1;
    }

    cont_state_t *state = (cont_state_t *)maple_dev_status(dev);
    if (!state) {
        return (uw)-1;
    }

    uw b = (uw)(state->buttons & 0xffff);
    if (state->joyx < -DC_STICK_THRESHOLD) b |= CONT_DPAD_LEFT;
    if (state->joyx >  DC_STICK_THRESHOLD) b |= CONT_DPAD_RIGHT;
    if (state->joyy < -DC_STICK_THRESHOLD) b |= CONT_DPAD_UP;
    if (state->joyy >  DC_STICK_THRESHOLD) b |= CONT_DPAD_DOWN;
    return b;
}

static const char *mediaTag(MediaKind kind) {
    switch (kind) {
        case MEDIA_EXE:  return " [EXE]";
        case MEDIA_DISC: return " [DISC]";
        default:         return "";
    }
}

int menuPickGame(const MediaEntry *items, int count) {
    menuInitFont();

    const int total = count + 1; // + trailing "Start BIOS" entry
    int sel = 0;
    int idleFrames = 0;
    uw prev = (uw)-1; // Swallow buttons already held on entry

    glViewport(0, 0, DC_SCREEN_W, DC_SCREEN_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DC_SCREEN_W, DC_SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (;;) {
        // --- Input -----------------------------------------------------
        uw cur = menuButtons();
        if (cur == (uw)-1) {
            // No controller: boot the first entry after a grace period
            if (++idleFrames > MENU_TIMEOUT) {
                return count ? 0 : -1;
            }
            cur = prev == (uw)-1 ? 0 : prev;
        }
        else {
            idleFrames = 0;
        }

        uw pressed = prev == (uw)-1 ? 0 : (cur & ~prev);
        prev = cur;

        if (pressed & CONT_DPAD_UP)   sel = (sel + total - 1) % total;
        if (pressed & CONT_DPAD_DOWN) sel = (sel + 1) % total;
        if (pressed & CONT_DPAD_LEFT)  sel = (sel + total - MENU_VISIBLE) % total;
        if (pressed & CONT_DPAD_RIGHT) sel = (sel + MENU_VISIBLE) % total;
        if (pressed & (CONT_A | CONT_START)) {
            return sel < count ? sel : -1;
        }
        if (pressed & CONT_B) {
            return -1;
        }

        // --- Render ----------------------------------------------------
        glClearColor(0.04f, 0.04f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, fontTex);

        glColor4ub(255, 255, 255, 255);
        drawText(40, 32, "PSeudo - Select media");

        // Keep the selection inside the visible window
        int first = sel - MENU_VISIBLE / 2;
        if (first > total - MENU_VISIBLE) first = total - MENU_VISIBLE;
        if (first < 0) first = 0;

        for (int row = 0; row < MENU_VISIBLE && first + row < total; row++) {
            int i = first + row;
            int y = 88 + row * (FONT_CH + 4);

            const char *base = baseName(items[i].path);
            char label[MEDIA_PATH_MAX + 12];
            snprintf(label, sizeof(label), "%s%s", base, mediaTag(items[i].kind));

            if (i == sel) {
                glColor4ub(255, 220, 80, 255);
                drawText(40, y, ">");
            }
            else {
                glColor4ub(170, 170, 170, 255);
            }
            drawText(64, y, label);
        }

        glColor4ub(120, 120, 140, 255);
        drawText(40, 440, "DPAD move  L/R page  A/START boot  B BIOS");

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        glKosSwapBuffers();
    }
}
#endif // __KOS__
