# PSeudo unit tests — build and run on the host, reusing the stub
# KOS/GLdc/ALdc headers from the Dreamcast host-check build so the whole
# emulator core links without real GL/AL/BIOS dependencies.
#
#   make -C Tests && ./Tests/gte-test

SOURCE_DIR := ../Source
DC_DIR     := ../Platforms/Dreamcast

CORE_SRCS := Bus CD Counters Disc Draw GTE Graphics Hardware MDEC Mem \
             PSeudo R3000A SIO Sound TextureCache

CXX      ?= g++
CXXFLAGS += -std=gnu++17 -Wall -O2 -DDREAMCAST -D__KOS__ \
            -I$(DC_DIR)/HostCheck -I$(SOURCE_DIR) -Wno-changes-meaning

all: gte-test

gte-test: GTE.cc $(DC_DIR)/HostCheck/Stubs.cc \
          $(addprefix $(SOURCE_DIR)/,$(addsuffix .cc,$(CORE_SRCS)))
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: gte-test
	./gte-test

clean:
	-rm -f gte-test

.PHONY: all run clean
