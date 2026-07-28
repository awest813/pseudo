# pseudoXDC (psxdc)

**pseudoXDC** (short: **psxdc**) is the Sega Dreamcast port of
[PSeudo](https://github.com/dkoluris/pseudo), a lightweight PlayStation
emulator. It runs on real Dreamcast hardware via
[KallistiOS](https://github.com/KallistiOS/KallistiOS), with GLdc for
video and ALdc for audio.

## Goal

Ship a practical PS1 experience on Dreamcast by steadily improving:

| Focus | What we’re chasing |
|-------|--------------------|
| **Compatibility** | Boot and play more commercial titles (e.g. 2.5D / FMV-heavy games) with fewer freezes, missing graphics, or silent audio |
| **Performance** | Keep the SH4 busy on useful work — idle-skip, tighter hot paths, sensible audio buffering |
| **Features** | Disc formats, XA/CD audio, memcards, MDEC/FMV, a usable on-console boot UI |

Upstream PSeudo is the core. This tree extends that core and the Dreamcast
platform layer toward those three goals.

## Progress

Rough status relative to a playable Dreamcast build (not a claim that
any specific commercial title is finished):

| Area | Status | Notes |
|------|--------|--------|
| CPU (R3000A) | Strong | Interpreter core; DC idle-skip for vsync poll loops |
| GTE | Improved | UNR divide, SF/LM on lighting & RTPS/RTPT, MVMVA edge cases |
| GPU | Partial | STP/mask, VRAM fill, texture window, GPUSTAT mirrors; still GL-immediate, not a full PS1 rasterizer |
| CD-ROM | Improved | MODE1/2048 & raw 2352, ReadN/ReadS, GetlocL, XA filter, sector delivery fixes |
| XA audio | Present | ADPCM decode + 37800→44100 resample into the SPU mix |
| SPU | Partial | Key on/off, ENDX, ENVX stub; no full ADSR/reverb yet |
| MDEC | Present | Status/protocol work for FMV; needs more real-stream validation |
| Memcard / SIO | Present | 128KB card, pad map, `/pc/memcard1.mcr` when available |
| Rootcounters | Improved | Once/repeat IRQ, pulse/toggle |
| Boot UI | Present | Branded media picker (scroll, key-repeat, confirm, fatal BIOS screen) |

**Still open for title-class play:** GPU accuracy (blend/textures), CD
streaming timing, SPU envelopes, and real hardware smoke tests (e.g.
toward games like Klonoa).

## Building

**Dreamcast binary** — source the KOS environment, then:

```
make -C Platforms/Dreamcast        # produces pseudo-dreamcast.elf
```

Requires kos-ports `libGL` (GLdc) and `libAL` (ALdc). CI builds the ELF
in `ghcr.io/kos-builds/kos-ports-dc` and uploads it as an artifact.

**Host check** — without KOS, the same `make` links the emulator against
stub headers in `HostCheck/` so PC CI catches porting breaks.

**Unit tests** (host):

```
make -C Tests run
```

## Media

Content is detected by type, not filename. Scan order: romdisk `/rd`,
then GD-ROM `/cd` (plus optional dc-load path args).

| Media | Detection |
|-------|-----------|
| PS1 BIOS | Any 512KB dump, e.g. `SCPH1001.BIN` (**not distributed**) |
| Game | Raw disc image (2352 or 2048 bytes/sector), or a `.cue` naming one |
| | ECM images are detected but unsupported — decompress first |
| Homebrew | `PS-X EXE` file |

Files in `romdisk/` bake into `/rd`. Prefer the GD-ROM for full-size
images (romdisk is loaded into RAM).

With a BIOS present, the **pseudoXDC** boot menu always appears (even
for one game, so you can still open the BIOS shell). Hold D-pad to
scroll, **A**/**Start** to boot, **B** for BIOS. No controller → short
countdown. Missing BIOS → on-screen fatal notice.

## Controls

| Dreamcast | PlayStation |
|-----------|-------------|
| D-pad / analog stick | D-pad |
| A | Cross |
| B | Circle |
| X | Square |
| Y | Triangle |
| L trigger | L1 |
| R trigger | R1 |
| Z / C (arcade sticks) | L2 / R2 |
| Start | Start |
| L + R triggers + Start | Select |
| A + B + X + Y + Start | quit |

## Saves

Memory card data goes to `/pc/memcard1.mcr` when a `/pc` filesystem is
available (SD adapter, dcload host folder, etc.). Without `/pc`, the
card starts empty each run.

## Credits

- **PSeudo** — original emulator by [Dennis Koluris](https://naden.co) / [dkoluris/pseudo](https://github.com/dkoluris/pseudo)
- **pseudoXDC / psxdc** — Dreamcast port and ongoing compatibility, performance, and feature work in this tree
- **KallistiOS**, **GLdc**, **ALdc** — Dreamcast toolchain and APIs

Apache 2.0 — see the repository `LICENSE`.
