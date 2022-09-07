#include "meters.hpp"
#include "plugin.hpp"
#include "widgets.hpp"

#define GAIN_DEBUG

struct GAIN : Module {

    Meter leftMeter;
    Meter rightMeter;

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
        kLeftInput,
        kRightInput,

        kInputsLen
    };

    enum OutputId {
        kLeftOutput,
        kRightOutput,

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

        configInput(kLeftInput, "Left");
        configInput(kRightInput, "Right");

        configOutput(kLeftOutput, "Left");
        configOutput(kRightOutput, "Right");

        // TODO
        // configBypass(kLeftInput, kLeftOutput);

#ifdef GAIN_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void process(const ProcessArgs& args) override {

        float left = inputs[kLeftInput].getVoltage();
        float right = inputs[kRightInput].getVoltage();

        leftMeter.process(left, args.sampleTime);
        rightMeter.process(right, args.sampleTime);

#ifdef GAIN_DEBUG
        outputs[kDebug1].setVoltage(leftMeter.rms);
        outputs[kDebug2].setVoltage(leftMeter.peak);
        outputs[kDebug3].setVoltage(rightMeter.rms);
        outputs[kDebug4].setVoltage(rightMeter.peak);
#endif

        outputs[kLeftOutput].setVoltage(left);
        outputs[kRightOutput].setVoltage(right);

        outputs[kLeftOutput].setChannels(1);
        outputs[kRightOutput].setChannels(1);
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

        addInput(createInputCentered<MPort>(Vec(22.5, 248), module, GAIN::kLeftInput));
        addInput(createInputCentered<MPort>(Vec(22.5, 276), module, GAIN::kRightInput));
        addOutput(createOutputCentered<MPort>(Vec(22.5, 306), module, GAIN::kLeftOutput));
        addOutput(createOutputCentered<MPort>(Vec(22.5, 334), module, GAIN::kRightOutput));
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
