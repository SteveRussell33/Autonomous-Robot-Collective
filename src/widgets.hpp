#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct RmPort24 : SvgPort {
    RmPort24() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/port24.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};

struct RmKnob : RoundKnob {
    RmKnob(const char* svg, int dim) {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
        box.size = Vec(dim, dim);
    }
};

struct RmKnob24 : RmKnob {
    RmKnob24() : RmKnob("res/knob24.svg", 24) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmToggleButton : SvgSwitch {
    RmToggleButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-1.svg")));
        box.size = Vec(18, 18);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};
