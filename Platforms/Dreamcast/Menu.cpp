// Boot menu for the Dreamcast port: lists media found on the romdisk /
// GD-ROM and lets the player pick with the controller.
// Text is drawn with a GL texture atlas built from the Dreamcast's
// BIOS font (bfont), so no font asset ships with the binary.

#include "Global.h"

#ifdef __KOS__
#include "../../Source/Global.h"
#include "Menu.h"

#include <dc/biosfont.h>
#include <math.h>

// bfont glyphs are 12x24, 1bpp, 3 bytes per 2 rows
#define FONT_CW    12
#define FONT_CH    24
#define FONT_FIRST 32
#define FONT_LAST  126
#define FONT_COLS  16

#define ATLAS_W 256
#define ATLAS_H 256

#define MENU_VISIBLE  10
#define MENU_TIMEOUT (10 * 60) // frames without a controller before auto-boot

// Visual direction: deep ink + warm amber (not purple / cream / glow stacks)
#define COL_INK_R     10
#define COL_INK_G     16
#define COL_INK_B     22
#define COL_TEAL_R    18
#define COL_TEAL_G    42
#define COL_TEAL_B    48
#define COL_BRAND_R   236
#define COL_BRAND_G   232
#define COL_BRAND_B   220
#define COL_ACCENT_R  232
#define COL_ACCENT_G  168
#define COL_ACCENT_B  64
#define COL_MUTED_R   120
#define COL_MUTED_G   138
#define COL_MUTED_B   142
#define COL_ROW_R     188
#define COL_ROW_G     196
#define COL_ROW_B     192

static GLuint fontTex = 0;

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

static void drawFill(float x0, float y0, float x1, float y1,
                     ub r, ub g, ub b, ub a) {
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

static void drawTextScaled(float x, float y, float scale, const char *s) {
    const float cw = FONT_CW * scale;
    const float ch = FONT_CH * scale;

    glBegin(GL_QUADS);
    for (; *s; s++, x += cw) {
        unsigned char c = (unsigned char)*s;
        if (c < FONT_FIRST || c > FONT_LAST) {
            continue;
        }
        if (c == ' ') {
            continue;
        }
        int i = c - FONT_FIRST;
        float u0 = ((i % FONT_COLS) * FONT_CW) / (float)ATLAS_W;
        float v0 = ((i / FONT_COLS) * FONT_CH) / (float)ATLAS_H;
        float u1 = u0 + FONT_CW / (float)ATLAS_W;
        float v1 = v0 + FONT_CH / (float)ATLAS_H;

        glTexCoord2f(u0, v0); glVertex2f(x,      y);
        glTexCoord2f(u1, v0); glVertex2f(x + cw, y);
        glTexCoord2f(u1, v1); glVertex2f(x + cw, y + ch);
        glTexCoord2f(u0, v1); glVertex2f(x,      y + ch);
    }
    glEnd();
}

static void drawText(int x, int y, const char *s) {
    drawTextScaled((float)x, (float)y, 1.0f, s);
}

static const char *baseName(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

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

static const char *mediaKindLabel(MediaKind kind) {
    switch (kind) {
        case MEDIA_EXE:  return "EXE ";
        case MEDIA_DISC: return "DISC";
        case MEDIA_BIOS: return "BIOS";
        default:         return "----";
    }
}

static void drawBackdrop(int frame) {
    // Slow teal breathe in the lower half — atmosphere without glow spam
    const float breathe = 0.5f + 0.5f * sinf(frame * 0.012f);
    const int tealLift = (int)(6.0f * breathe);

    glDisable(GL_TEXTURE_2D);
    const int bands = 16;
    for (int i = 0; i < bands; i++) {
        float t = i / (float)(bands - 1);
        ub r = (ub)(COL_INK_R + (COL_TEAL_R + tealLift - COL_INK_R) * t);
        ub g = (ub)(COL_INK_G + (COL_TEAL_G + tealLift - COL_INK_G) * t);
        ub b = (ub)(COL_INK_B + (COL_TEAL_B - COL_INK_B) * t);
        float y0 = (DC_SCREEN_H * i) / (float)bands;
        float y1 = (DC_SCREEN_H * (i + 1)) / (float)bands;
        drawFill(0, y0, DC_SCREEN_W, y1, r, g, b, 255);
    }

    // Quiet brand plane behind the title — not a card, just a soft field
    drawFill(0, 0, DC_SCREEN_W, 118, COL_INK_R, COL_INK_G, COL_INK_B, 180);
    // Thin amber rule under the brand block
    drawFill(40, 108, 200, 110, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, 255);
}

int menuPickGame(const MediaEntry *items, int count) {
    menuInitFont();

    const int total = count + 1; // trailing "Start BIOS"
    int sel = 0;
    int idleFrames = 0;
    int frame = 0;
    uw prev = (uw)-1; // Swallow buttons already held on entry

    glViewport(0, 0, DC_SCREEN_W, DC_SCREEN_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DC_SCREEN_W, DC_SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (;;) {
        frame++;

        // --- Input -----------------------------------------------------
        uw cur = menuButtons();
        bool noPad = (cur == (uw)-1);
        if (noPad) {
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
        glClearColor(COL_INK_R / 255.0f, COL_INK_G / 255.0f, COL_INK_B / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        drawBackdrop(frame);

        // Intro fade for brand (first ~40 frames)
        ub brandA = 255;
        if (frame < 40) {
            brandA = (ub)((frame * 255) / 40);
        }

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fontTex);

        glColor4ub(COL_BRAND_R, COL_BRAND_G, COL_BRAND_B, brandA);
        drawTextScaled(40, 28, 2.0f, "PSeudo");

        glColor4ub(COL_MUTED_R, COL_MUTED_G, COL_MUTED_B, brandA);
        drawText(40, 82, "Choose something to boot");

        // Keep the selection inside the visible window
        int first = sel - MENU_VISIBLE / 2;
        if (first > total - MENU_VISIBLE) first = total - MENU_VISIBLE;
        if (first < 0) first = 0;

        const float pulse = 0.55f + 0.45f * sinf(frame * 0.09f);
        const int railA = (int)(140 + 100 * pulse);

        for (int row = 0; row < MENU_VISIBLE && first + row < total; row++) {
            int i = first + row;
            int y = 128 + row * (FONT_CH + 8);

            char kind[8];
            char label[MEDIA_PATH_MAX];
            if (i < count) {
                snprintf(kind, sizeof(kind), "%s", mediaKindLabel(items[i].kind));
                snprintf(label, sizeof(label), "%s", baseName(items[i].path));
            }
            else {
                snprintf(kind, sizeof(kind), "BIOS");
                snprintf(label, sizeof(label), "Start BIOS shell");
            }

            const bool on = (i == sel);
            float xOff = on ? (3.0f + 2.0f * sinf(frame * 0.11f)) : 0.0f;

            glDisable(GL_TEXTURE_2D);
            if (on) {
                // Selection rail — one accent mark, not a card chrome stack
                drawFill(40, (float)y, 44, (float)(y + FONT_CH),
                         COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, (ub)railA);
                drawFill(48, (float)y - 2, DC_SCREEN_W - 40, (float)(y + FONT_CH + 2),
                         COL_TEAL_R, COL_TEAL_G, COL_TEAL_B, 70);
            }
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, fontTex);

            if (on) {
                glColor4ub(COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, 255);
            }
            else {
                glColor4ub(COL_MUTED_R, COL_MUTED_G, COL_MUTED_B, 255);
            }
            drawTextScaled(56 + xOff, (float)y, 1.0f, kind);

            if (on) {
                glColor4ub(COL_BRAND_R, COL_BRAND_G, COL_BRAND_B, 255);
            }
            else {
                glColor4ub(COL_ROW_R, COL_ROW_G, COL_ROW_B, 255);
            }
            drawTextScaled(120 + xOff, (float)y, 1.0f, label);
        }

        // Footer: one job — controls / timeout
        glDisable(GL_TEXTURE_2D);
        drawFill(0, 430, DC_SCREEN_W, DC_SCREEN_H, COL_INK_R, COL_INK_G, COL_INK_B, 210);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fontTex);
        glColor4ub(COL_MUTED_R, COL_MUTED_G, COL_MUTED_B, 255);

        if (noPad) {
            char wait[64];
            int left = (MENU_TIMEOUT - idleFrames + 59) / 60;
            if (left < 0) left = 0;
            snprintf(wait, sizeof(wait), "No controller — booting in %ds", left);
            drawText(40, 444, wait);

            glDisable(GL_TEXTURE_2D);
            float progress = idleFrames / (float)MENU_TIMEOUT;
            if (progress > 1.0f) progress = 1.0f;
            drawFill(40, 472, 40 + progress * (DC_SCREEN_W - 80), 476,
                     COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, 220);
        }
        else {
            drawText(40, 444, "D-pad move   A / Start boot   B BIOS");
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        glKosSwapBuffers();
    }
}
#endif // __KOS__
