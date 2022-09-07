#include "dsp.hpp"
#include "plugin.hpp"
#include "widgets.hpp"
#include "vu.hpp"

// define GAIN_DEBUG

struct GAIN : Module {

    // GAIN is mono, but its easier to pretend that its stereo 
    // and has no right input connected.
    Levels leftLevels;
    Levels rightLevels;

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

        float in = inputs[kInput].getVoltage();

        // GAIN is mono, but its easier to pretend that its stereo 
        // and has no right input connected.
        leftLevels.process(in, args.sampleTime);
        rightLevels.rms = leftLevels.rms;
        rightLevels.peak = leftLevels.peak;

#ifdef GAIN_DEBUG
        outputs[kDebug1].setVoltage(leftLevels.rms);
        outputs[kDebug2].setVoltage(leftLevels.peak);
#endif

        outputs[kOutput].setVoltage(in);
    }
};

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

        VuMeter *meter = createWidget<VuMeter>(Vec(22.5-9, 40));
        //meter->setLevels(leftLevels, rightLevels);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
