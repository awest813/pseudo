#ifndef PSEUDO_DREAMCAST_GLOBAL_H
#define PSEUDO_DREAMCAST_GLOBAL_H

#ifdef __KOS__
#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#else
#include <cstdio>
#include <cstdlib>
#endif

#define PSEUDO_DREAMCAST_PLATFORM "Dreamcast"

// Dreamcast display resolution
#define DC_SCREEN_W 640
#define DC_SCREEN_H 480

// Analog trigger threshold for digital button conversion (0-255 range).
// Values above this threshold are treated as "pressed".
#define DC_LTRIG_THRESHOLD 64
#define DC_RTRIG_THRESHOLD 64

// Synthetic button bits used to pass L/R trigger state through padListener.
// These are placed above the 16-bit CONT_* bitmask range so they never
// alias real Dreamcast button bits.
#define DC_BTN_LTRIG (1 << 16)
#define DC_BTN_RTRIG (1 << 17)

#endif
