# PSeudo → pseudoXDC (psxdc)

This repository contains **pseudoXDC** (also called **psxdc**): a
**Sega Dreamcast** port of the [PSeudo](https://github.com/dkoluris/pseudo)
PlayStation emulator.

**Primary docs for the Dreamcast build:**
[Platforms/Dreamcast/README.md](Platforms/Dreamcast/README.md)

### What this fork is for

Increase **compatibility**, **performance**, and **features** so more
PS1 software can boot and play on real Dreamcast hardware — CD/XA,
GTE/GPU accuracy, audio, memcards, FMV/MDEC, and a usable on-console
UI — without giving up PSeudo’s small-core approach.

---

# Upstream PSeudo

The sections below describe the original **PSeudo** project
([naden.co](https://naden.co)). Desktop macOS/Windows builds and the
historical completion list are upstream’s; Dreamcast-specific status
lives in the [Dreamcast README](Platforms/Dreamcast/README.md).

[![GitHub stars](https://img.shields.io/github/stars/dkoluris/pseudo.svg?style=flat-square)](https://github.com/dkoluris/pseudo/stargazers) [![GitHub license](https://img.shields.io/github/license/dkoluris/pseudo.svg?style=flat-square)](https://github.com/dkoluris/pseudo/blob/master/LICENSE)

**PSeudo aims to be the world's simplest PSX emulator** both in terms of user experience and software implementation. It is coded in **C/C++** and is utilizing modern APIs such as **OpenGL** for visuals and **OpenAL** for audio. The original author developed it primarily on **macOS**, with Windows experiments as well.

<a href="https://github.com/dkoluris/pseudo/releases/tag/version-0.87">>> Get the latest upstream PSeudo 0.87 release for Mac <<</a>

## Showcase

<img alt="Gran Turismo 2" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/turismo.jpg" width="48.5%"/><img alt="WipEout 3" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/wipeout-3.jpg" width="48.5%" align="right"/>

<img alt="Suikoden II" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/suikoden.jpg" width="48.5%"/><img alt="Crash Bandicoot" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/crash-bandicoot.jpg" width="48.5%" align="right"/>

## Upstream completion (historical)

Here's a list with the overall progress of the emulator, broken down in distinct parts (upstream snapshot; the Dreamcast tree has moved several of these forward — see the Dreamcast README).

* `95% -> CPU Mips R3000A`
* `90% -> DMA`
* `85% -> Mem IO`
* `85% -> Movie Decoder`
* `80% -> Interrupts`
* `75% -> CD Decoder`
* `70% -> GPU Primitives & Commands`
* `65% -> GPU Textures`
* `60% -> Audio`
* `65% -> GTE`
* `60% -> Rootcounters`
* `25% -> Serial IO`
* `10% -> XA Audio`

**PSeudo** can load some commercial games, but it needs work on performance and overall timing.

<img alt="Tekken 3" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/tekken.jpg" width="48.5%"/><img alt="Ridge Racer Type-4" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/racer.jpg" width="48.5%" align="right"/>

## Compile / Build (upstream desktop)

In order to build the application on **macOS**, the prerequisite is Xcode 10+. For **Windows** install **Microsoft Visual Studio 2019** and on top the additional **Clang** compiler, we will use that instead of MSVC's default. This is important in order to keep the source code identical between different platforms.

For the Dreamcast / **pseudoXDC** build, see
[Platforms/Dreamcast/README.md](Platforms/Dreamcast/README.md).

<img alt="Gran Turismo 2" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/turismo-2.jpg" width="48.5%"/><img alt="WipEout XL" src="https://raw.githubusercontent.com/dkoluris/pseudo/master/Resources/wipeout-xl.jpg" width="48.5%" align="right"/>

## License

Open-source under [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).
