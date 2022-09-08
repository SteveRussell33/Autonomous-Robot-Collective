
DSP_SOURCES = $(wildcard lib/bogaudio/dsp/*.cpp lib/bogaudio/dsp/filters/*.cpp)
SOURCES = $(wildcard src/*.cpp lib/earlevel/*.cpp) $(DSP_SOURCES)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

CXXFLAGS += -Isrc -Isrc/dsp -Ilib/earlevel -Ilib
