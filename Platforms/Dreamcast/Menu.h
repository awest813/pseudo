#ifndef PSEUDO_DREAMCAST_MENU_H
#define PSEUDO_DREAMCAST_MENU_H

// Media found on the romdisk / GD-ROM, shared between the discovery
// scan (Main.cpp) and the boot menu (Menu.cpp)

#define MEDIA_PATH_MAX    256
#define MEDIA_MAX_ENTRIES 24

enum MediaKind {
    MEDIA_NONE,
    MEDIA_BIOS, // 512KB raw BIOS dump
    MEDIA_EXE,  // "PS-X EXE" header
    MEDIA_DISC, // raw disc image, 2352 bytes per sector
    MEDIA_CUE,  // cue sheet naming a disc image
};

struct MediaEntry {
    char path[MEDIA_PATH_MAX];
    MediaKind kind;
};

#ifdef __KOS__
// Show the boot picker for the given media list. Returns the index of
// the chosen entry, or -1 for the BIOS shell. With no controller
// plugged in, the first entry is chosen after a ~10 second timeout.
int menuPickGame(const MediaEntry *items, int count);
#endif

#endif
