#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct MPolyPort : SvgPort {
    MPolyPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/poly-port.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

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

struct MKnob24 : MKnob {
    MKnob24() : MKnob("res/knob24.svg", 24) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct MKnob45 : MKnob {
    MKnob45() : MKnob("res/knob45.svg", 45) {
        shadow->blurRadius = 2.5;
        shadow->box.pos = Vec(0.0, 3.5);
    }
};

struct MHSwitch : SvgSwitch {
    MHSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-1.svg")));
    }
};

struct MToggleButton : SvgSwitch {
    MToggleButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-1.svg")));
        box.size = Vec(18, 18);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};
