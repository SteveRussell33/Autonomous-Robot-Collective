
LIB_SOURCES = $(wildcard lib/bogaudio/dsp/*.cpp lib/bogaudio/dsp/filters/*.cpp lib/earlevel/*.cpp)
SOURCES = $(wildcard src/*.cpp) $(LIB_SOURCES)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

# NOTE: -Ilib/bogaudio/dsp is there for BogAudio to compile successfully
CXXFLAGS += -Isrc -Isrc/dsp -Ilib -Ilib/bogaudio/dsp
