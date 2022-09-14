#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

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

struct RmKnob32 : RmKnob {
    RmKnob32() : RmKnob("res/knob32.svg", 32) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
    }
};

struct RmKnob40 : RmKnob {
    RmKnob40() : RmKnob("res/knob40.svg", 40) {
        shadow->blurRadius = 2.0;
        shadow->box.pos = Vec(0.0, 3.0);
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

struct RmFader : SvgSlider {
    RmFader() {
        setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-bg.svg")));
        setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-handle.svg")));
        setHandlePos(
            Vec(0, 143 - 0.01), // nudge to keep it from disappearing
            Vec(0, 0.5));
        box.size = Vec(12, 160);
    }
};
