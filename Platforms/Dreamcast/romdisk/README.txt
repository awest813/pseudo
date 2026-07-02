Files in this directory are baked into the KOS romdisk and appear under
/rd at runtime.

Put the PS1 BIOS here as SCPH1001.BIN (not distributed with PSeudo), and
optionally a game image as PSX.BIN or GAME.BIN. Without a game image the
emulator boots into the BIOS shell; without a BIOS it looks for the same
files on the GD-ROM (/cd) and exits if none are found.
