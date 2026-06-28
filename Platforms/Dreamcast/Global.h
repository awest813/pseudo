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

#endif
