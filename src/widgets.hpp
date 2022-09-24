#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct ArcPolyPort : SvgPort {
    ArcPolyPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/poly-port.svg")));
        box.size = Vec(24, 24);
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct ArcKnob : RoundKnob {
    ArcKnob(const char* svg, int dim) {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
        box.size = Vec(dim, dim);
    }
};

struct ArcKnob18 : ArcKnob {
    ArcKnob18() : ArcKnob("res/knob18.svg", 18) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct ArcKnob24 : ArcKnob {
    ArcKnob24() : ArcKnob("res/knob24.svg", 24) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct ArcKnob45 : ArcKnob {
    ArcKnob45() : ArcKnob("res/knob45.svg", 45) {
        shadow->blurRadius = 2.5;
        shadow->box.pos = Vec(0.0, 3.5);
    }
};

struct ArcHSwitch : SvgSwitch {
    ArcHSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-1.svg")));
    }
};

struct ArcMuteButton : SvgSwitch {
    ArcMuteButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-gray.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-orange.svg")));
        box.size = Vec(18, 18);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};

