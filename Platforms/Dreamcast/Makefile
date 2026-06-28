KOS_BASE   ?= /opt/dc/kos
TARGET     := pseudo-dreamcast
SOURCE_DIR := ../../Source

CXX ?= g++

ifeq ($(wildcard $(KOS_BASE)/bin/sh-elf-g++),$(KOS_BASE)/bin/sh-elf-g++)
# ---------------------------------------------------------------------------
# KOS (Dreamcast) build
# ---------------------------------------------------------------------------
CXX      := $(KOS_BASE)/bin/sh-elf-g++
CXXFLAGS += -DDREAMCAST \
            -I$(KOS_BASE)/include \
            -I$(KOS_BASE)/../kos-ports/include \
            -I$(SOURCE_DIR) \
            -I.
LDFLAGS  += -L$(KOS_BASE)/lib
TARGET   := $(TARGET).elf
SOURCES  := Main.cpp \
            $(SOURCE_DIR)/Bus.cc \
            $(SOURCE_DIR)/CD.cc \
            $(SOURCE_DIR)/Counters.cc \
            $(SOURCE_DIR)/Disc.cc \
            $(SOURCE_DIR)/Draw.cc \
            $(SOURCE_DIR)/GTE.cc \
            $(SOURCE_DIR)/Graphics.cc \
            $(SOURCE_DIR)/Hardware.cc \
            $(SOURCE_DIR)/MDEC.cc \
            $(SOURCE_DIR)/Mem.cc \
            $(SOURCE_DIR)/PSeudo.cc \
            $(SOURCE_DIR)/R3000A.cc \
            $(SOURCE_DIR)/SIO.cc \
            $(SOURCE_DIR)/Sound.cc \
            $(SOURCE_DIR)/TextureCache.cc
else
# ---------------------------------------------------------------------------
# Host build (for basic compilation checks without the KOS toolchain)
# ---------------------------------------------------------------------------
CXXFLAGS += -std=c++17 -Wall -Wextra -I.
TARGET   := $(TARGET).host
SOURCES  := Main.cpp
endif

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
