#include "dsp.hpp"
#include "plugin.hpp"
#include "widgets.hpp"
// include "vu.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    VULevels vuLevels;

#ifdef GAIN_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {

        kParamsLen
    };

    enum InputId {
        kVolInput,
        kInput,

        kInputsLen
    };

    enum OutputId {
        kOutput,

#ifdef GAIN_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    GAIN() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configInput(kVolInput, "Vol");
        configInput(kInput, "Audio");

        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef GAIN_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void process(const ProcessArgs& args) override {
        if (!outputs[kOutput].isConnected()) {
            return;
        }

        float in = inputs[kInput].getVoltage();

        vuLevels.process(in, in, args.sampleTime);

        outputs[kOutput].setVoltage(in);
    }
};

//--------------------------------------------------------------
// VUMeter
//--------------------------------------------------------------

struct VUMeter : OpaqueWidget {

    const NVGcolor red = nvgRGB(0xE6, 0x29, 0x34);
    const NVGcolor orange = nvgRGB(0xFF, 0x87, 0x24);
    const NVGcolor yellow = nvgRGB(0xFF, 0xCA, 0x33);
    const NVGcolor green = nvgRGB(0x3E, 0xD5, 0x64);

    // TODO make a base class that we can get VULevels from
    GAIN* module;

    VUMeter(GAIN* module_) : module(module_) {}

    void draw(const DrawArgs& args) override {

        float leftRms = 0.0;
        float rightRms = 0.0;
        if (module) {
            leftRms = module->vuLevels.leftRms;
            rightRms = module->vuLevels.rightRms;
        }

        drawLevel(args, 0, leftRms);
        drawLevel(args, 5, rightRms);
    }

    void drawLevel(const DrawArgs& args, float x, float level) {

        float db = clamp(ampToDb(level / 10.0f), -120.0f, 6.0f);
        if (db < -72.0f) {
            return;
        }

        NVGpaint orangeYellow = nvgLinearGradient(args.vg, 0, 30, 0, 45, orange, yellow);
        NVGpaint yellowGreen = nvgLinearGradient(args.vg, 0, 60, 0, 81, yellow, green);

        drawSegment(args, x, db, 3.0f, 6.0f, 30, 0, red, NVGpaint{}, true);
        drawSegment(args, x, db, 0.0f, 3.0f, 30, 15, orange, NVGpaint{}, true);
        drawSegment(args, x, db, -3.0f, 0.0f, 45, 30, NVGcolor{}, orangeYellow, false);
        drawSegment(args, x, db, -6.0f, -3.0f, 60, 45, yellow, NVGpaint{}, true);
        drawSegment(args, x, db, -12.0f, -6.0f, 81, 60, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, db, -24.f, -12.0f, 102, 81, green, NVGpaint{}, true);
        drawSegment(args, x, db, -48.f, -24.0f, 123, 102, green, NVGpaint{}, true);
        drawSegment(args, x, db, -72.f, -48.0f, 144, 123, green, NVGpaint{}, true);
    }

    void drawSegment(
        const DrawArgs& args,
        float x,
        float db,
        float lowDb,
        float highDb,
        float bottom,
        float top,
        NVGcolor color,
        NVGpaint gradient,
        bool isColor) {

        if (db < lowDb) {
            return;
        }

        float y = (db > highDb) ? top : rescale(db, lowDb, highDb, bottom, top);
        float height = bottom - y;
        drawRect(args, x, y, 3, height, color, gradient, isColor);
    }

    void drawRect(
        const DrawArgs& args,
        float x,
        float y,
        float w,
        float h,
        NVGcolor color,
        NVGpaint gradient,
        bool isColor) {

        nvgBeginPath(args.vg);
        nvgRect(args.vg, x, y, w, h);
        if (isColor) {
            nvgFillColor(args.vg, color);
        } else {
            nvgFillPaint(args.vg, gradient);
        }
        nvgFill(args.vg);
    }
};

//--------------------------------------------------------------
// GAINWidget
//--------------------------------------------------------------

struct GAINWidget : ModuleWidget {
    GAINWidget(GAIN* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GAIN.svg")));

        // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef GAIN_DEBUG
        addOutput(createOutputCentered<MPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<MPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<MPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<MPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        addInput(createInputCentered<MPort>(Vec(22.5, 240), module, GAIN::kVolInput));
        addInput(createInputCentered<MPort>(Vec(22.5, 279), module, GAIN::kInput));
        addOutput(createOutputCentered<MPort>(Vec(22.5, 320), module, GAIN::kOutput));

        VUMeter* meter = new VUMeter(module);
        meter->box.pos = Vec(22.5 - 9, 40);
        meter->box.size = Vec(9, 144);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
