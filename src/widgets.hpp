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
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-gray.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-orange.svg")));
        box.size = Vec(18, 18);
        shadow->blurRadius = 1.0;
        shadow->box.pos = Vec(0.0, 1.5);
    }
};

// Adapted from /github.com/bogaudio/BogaudioModules/src/mixer.cpp
//struct ArcSoloMuteButton : ParamWidget {
//    std::vector<std::shared_ptr<Svg>> _frames;
//    SvgWidget* _svgWidget; // deleted elsewhere.
//    CircularShadow* shadow = NULL;
//
//    ArcSoloMuteButton() {
//        shadow = new CircularShadow();
//        addChild(shadow);
//
//        _svgWidget = new SvgWidget();
//        addChild(_svgWidget);
//
//        auto svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-gray.svg"));
//        _frames.push_back(svg);
//        _frames.push_back(
//            APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-orange.svg")));
//        _frames.push_back(
//            APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-green.svg")));
//        _frames.push_back(
//            APP->window->loadSvg(asset::plugin(pluginInstance, "res/button-green.svg")));
//
//        _svgWidget->setSvg(svg);
//        box.size = _svgWidget->box.size;
//        shadow->box.size = _svgWidget->box.size;
//        shadow->blurRadius = 1.0;
//        shadow->box.pos = Vec(0.0, 1.0);
//    }
//
//    void reset() {
//        if (getParamQuantity()) {
//            getParamQuantity()->setValue(0.0f);
//        }
//    }
//
//    void randomize() {
//        if (getParamQuantity()) {
//            getParamQuantity()->setValue(random::uniform() > 0.5f ? 1.0f : 0.0f);
//        }
//    }
//
//    void onButton(const event::Button& e) {
//        if (!getParamQuantity() || !(e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0)) {
//            ParamWidget::onButton(e);
//            return;
//        }
//
//        float value = getParamQuantity()->getValue();
//        if (value >= 2.0f) {
//            getParamQuantity()->setValue(value - 2.0f);
//        } else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
//            getParamQuantity()->setValue(value + 2.0f);
//        } else {
//            getParamQuantity()->setValue(value > 0.5f ? 0.0f : 1.0f);
//        }
//
//        if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
//            e.consume(this);
//        } else {
//            ParamWidget::onButton(e);
//        }
//    }
//
//    void onChange(const event::Change& e) {
//        assert(_frames.size() == 4);
//        if (getParamQuantity()) {
//            float value = getParamQuantity()->getValue();
//            assert(value >= 0.0f && value <= 3.0f);
//            _svgWidget->setSvg(_frames[(int)value]);
//        }
//        ParamWidget::onChange(e);
//    }
//};
