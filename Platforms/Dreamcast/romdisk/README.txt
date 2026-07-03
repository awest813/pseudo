Files in this directory are baked into the KOS romdisk and appear under
/rd at runtime.

PSeudo detects media by content, not by file name, scanning /rd first
and the GD-ROM (/cd) second:

  - PS1 BIOS   any 512KB dump, e.g. SCPH1001.BIN (not distributed)
  - Game       a raw disc image (2352 bytes/sector .bin/.img), or a
               .cue sheet pointing at one
  - Homebrew   a PS-X EXE file

With a single game found it boots straight away; with several, a boot
menu lets you pick. Without any, the emulator boots into the BIOS
shell; without a BIOS it exits.

Note the romdisk is loaded into RAM whole, so prefer the GD-ROM for
full-size game images.
