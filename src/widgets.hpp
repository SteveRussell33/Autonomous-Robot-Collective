#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct RmPolyPort : SvgPort {
    RmPolyPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/poly-port.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmMonoPort : SvgPort {
    RmMonoPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/mono-port.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmKnob : RoundKnob {
    RmKnob(const char* svg, int dim) {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
        box.size = Vec(dim, dim);
    }
};

struct RmKnob18 : RmKnob {
    RmKnob18() : RmKnob("res/knob18.svg", 18) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmKnob24 : RmKnob {
    RmKnob24() : RmKnob("res/knob24.svg", 24) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmKnob45 : RmKnob {
    RmKnob45() : RmKnob("res/knob45.svg", 45) {
        shadow->blurRadius = 2.5;
        shadow->box.pos = Vec(0.0, 3.5);
    }
};

struct RmHSwitch : SvgSwitch {
    RmHSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-1.svg")));
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
