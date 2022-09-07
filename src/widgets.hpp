#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct MKnob : RoundKnob {
    MKnob(const char* svg, int dim) {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
        box.size = Vec(dim, dim);
    }
};

struct MKnob18 : MKnob {
    MKnob18() : MKnob("res/knob18.svg", 18) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct MKnob32 : MKnob {
    MKnob32() : MKnob("res/knob32.svg", 32) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct MKnob40 : MKnob {
    MKnob40() : MKnob("res/knob40.svg", 40) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct MPort : SvgPort {
    MPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};

struct MHSwitch : SvgSwitch {
    MHSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-1.svg")));
    }
};
