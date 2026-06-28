KOS_BASE ?= /opt/dc/kos
TARGET := pseudo-dreamcast
SOURCES := Main.cpp
CXX ?= g++

ifeq ($(wildcard $(KOS_BASE)/bin/sh-elf-g++),$(KOS_BASE)/bin/sh-elf-g++)
CXX := $(KOS_BASE)/bin/sh-elf-g++
CXXFLAGS += -I$(KOS_BASE)/include -I$(KOS_BASE)/../kos-ports/include
LDFLAGS += -L$(KOS_BASE)/lib
TARGET := $(TARGET).elf
else
CXXFLAGS += -std=c++17 -Wall -Wextra -I.
TARGET := $(TARGET).host
endif

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
