# PSeudo — Sega Dreamcast port
#
# Dreamcast build (produces pseudo-dreamcast.elf):
#   Source the KOS environment first (e.g. `. /opt/toolchains/dc/kos/environ.sh`),
#   then run `make`. Requires the kos-ports packages libGL (GLdc) and
#   libAL (ALdc). Files placed in romdisk/ (e.g. the SCPH1001.BIN BIOS)
#   are baked into the binary and show up under /rd at runtime.
#
# Host check build (produces pseudo-dreamcast.host):
#   Run `make` without the KOS environment. Compiles and links every
#   emulator source with -DDREAMCAST against the stub KOS/GLdc/ALdc
#   headers in HostCheck/, catching porting regressions on a PC.

SOURCE_DIR := ../../Source

CORE_SRCS := Bus CD Counters Disc Draw GTE Graphics Hardware MDEC Mem \
             PSeudo R3000A SIO Sound TextureCache

ifneq ($(wildcard $(KOS_BASE)/Makefile.rules),)
# ---------------------------------------------------------------------------
# KOS (Dreamcast) build
# ---------------------------------------------------------------------------
TARGET := pseudo-dreamcast.elf
OBJS   := Main.o $(addprefix $(SOURCE_DIR)/,$(addsuffix .o,$(CORE_SRCS))) \
          romdisk.o

KOS_ROMDISK_DIR := romdisk

# -Wno-changes-meaning: GTE.h fields legitimately shadow the uw/uh/ub
# typedef names, which GCC 13+ rejects in class scope by default
CXXFLAGS += -DDREAMCAST -I$(SOURCE_DIR) -I. -Wno-changes-meaning

all: $(TARGET)

include $(KOS_BASE)/Makefile.rules

$(TARGET): $(OBJS)
	kos-c++ -o $(TARGET) $(OBJS) -lGL -lAL

clean:
	-rm -f $(TARGET) $(OBJS) romdisk.img

.PHONY: all clean
else
# ---------------------------------------------------------------------------
# Host check build (no KOS toolchain)
# ---------------------------------------------------------------------------
CXX    ?= g++
TARGET := pseudo-dreamcast.host

SOURCES := Main.cpp HostCheck/Stubs.cc \
           $(addprefix $(SOURCE_DIR)/,$(addsuffix .cc,$(CORE_SRCS)))

CXXFLAGS += -std=gnu++17 -Wall -DDREAMCAST -D__KOS__ \
            -IHostCheck -I$(SOURCE_DIR) -I. -Wno-changes-meaning

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	-rm -f $(TARGET)

.PHONY: all run clean
endif
