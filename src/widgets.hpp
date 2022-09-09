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

struct MHSwitch : SvgSwitch {
    MHSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hswitch-1.svg")));
    }
};

struct MFader : SvgSlider {
	MFader() {
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-bg.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/fader-handle.svg")));
        setHandlePos(
            Vec(0, 144-0.1), // nudge to keep it from disappearing
            Vec(0, 0));
        box.size = Vec(12, 160);
	}
};

struct MToggleButton : SvgSwitch {
    MToggleButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/toggle-1.svg")));
        box.size = Vec(18,18);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};
