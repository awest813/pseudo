#ifndef PSEUDO_DREAMCAST_GLOBAL_H
#define PSEUDO_DREAMCAST_GLOBAL_H

// The KOS toolchain defines _arch_dreamcast / __DREAMCAST__, not __KOS__
#if (defined(_arch_dreamcast) || defined(__DREAMCAST__)) && !defined(__KOS__)
#define __KOS__ 1
#endif

#ifdef __KOS__
#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <GL/glkos.h>
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

// Analog stick deflection (-128..127) treated as a digital D-pad press
#define DC_STICK_THRESHOLD 64

// Synthetic button bits used to pass extra state through padListener.
// These are placed above the 16-bit CONT_* bitmask range so they never
// alias real Dreamcast button bits.
#define DC_BTN_LTRIG  (1 << 16)
#define DC_BTN_RTRIG  (1 << 17)
#define DC_BTN_SELECT (1 << 18) // No Select on DC pads: L+R triggers + Start

// Total number of button bits tracked by the polling loop
#define DC_BTN_BITS 19

#endif
