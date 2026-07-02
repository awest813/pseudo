/* Host-check stub of KallistiOS <dc/biosfont.h>.
 *
 * Real bfont glyphs are 12x24 pixels, 1bpp, 36 bytes per glyph
 * (3 bytes per 2 rows, high bits first). */
#ifndef PSEUDO_HOSTCHECK_DC_BIOSFONT_H
#define PSEUDO_HOSTCHECK_DC_BIOSFONT_H

#ifdef __cplusplus
extern "C" {
#endif

#define BFONT_THIN_WIDTH 12
#define BFONT_HEIGHT     24

unsigned char *bfont_find_char(unsigned int ch);

#ifdef __cplusplus
}
#endif

#endif
