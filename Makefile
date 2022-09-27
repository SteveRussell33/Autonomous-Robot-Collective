
LIB_SOURCES = $(wildcard lib/earlevel/*.cpp)
SOURCES = $(wildcard src/*.cpp) $(LIB_SOURCES)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

CXXFLAGS += -Isrc -Isrc/dsp -Ilib
