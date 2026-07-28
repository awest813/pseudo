# PSeudo — Sega Dreamcast port

A port of the PSeudo PSX emulator to the Sega Dreamcast, built on
[KallistiOS](https://github.com/KallistiOS/KallistiOS) with GLdc for
rendering and ALdc for audio.

## Building

**Dreamcast binary** — source the KOS environment, then:

```
make -C Platforms/Dreamcast        # produces pseudo-dreamcast.elf
```

Requires the kos-ports packages `libGL` (GLdc) and `libAL` (ALdc). CI
builds the ELF in the `ghcr.io/kos-builds/kos-ports-dc` container and
uploads it as an artifact.

**Host check** — without the KOS environment, the same `make` compiles
and links every emulator source against the stub KOS/GLdc/ALdc headers
in `HostCheck/`, catching porting regressions on a PC.

## Media

PSeudo detects media by content, not by file name, scanning the romdisk
(`/rd`) first and the GD-ROM (`/cd`) second:

| Media    | Detection                                              |
|----------|--------------------------------------------------------|
| PS1 BIOS | any 512KB dump, e.g. `SCPH1001.BIN` (not distributed)  |
| Game     | raw disc image (2352 or 2048 bytes/sector), or a `.cue` naming one |
| Homebrew | a PS-X EXE file                                        |

Files placed in `romdisk/` are baked into the binary at `/rd`. The
romdisk is loaded into RAM whole, so prefer the GD-ROM for full-size
game images.

With a single game found it boots straight away. With several, a boot
menu lists them (plus a *Start BIOS* entry): D-pad or analog stick to
move, **A**/**Start** to boot, **B** for the BIOS shell. Without a
controller the first entry boots after ten seconds.

## Controls

| Dreamcast              | PlayStation |
|------------------------|-------------|
| D-pad / analog stick   | D-pad       |
| A                      | Cross       |
| B                      | Circle      |
| X                      | Square      |
| Y                      | Triangle    |
| L trigger              | L1          |
| R trigger              | R1          |
| Z / C (arcade sticks)  | L2 / R2     |
| Start                  | Start       |
| L + R triggers + Start | Select      |
| A + B + X + Y + Start  | quit        |
