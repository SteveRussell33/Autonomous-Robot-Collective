#include "plugin.hpp"
#include "vu.hpp"
#include "widgets.hpp"

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
        kFader,
        kMute,

        kParamsLen
    };

    enum InputId {
        kLevelInput,
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

        configParam(kFader, 0.0f, 1.0f, 0.0f, "Fader");
        configParam(kMute, 0.0f, 1.0f, 0.0f, "Mute");

        configInput(kLevelInput, "Level");
        configInput(kLeftInput, "Left");
        configInput(kRightInput, "Right");

        configOutput(kLeftOutput, "Left");
        configOutput(kRightOutput, "Right");

        // I guess there is no bypass?
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

        float right = left;
        if (inputs[kRightInput].isConnected()) {
            right = inputs[kRightInput].getVoltage();
        }

        vuLevels.process(left, right, args.sampleTime);

        if (outputs[kLeftOutput].isConnected()) {
            outputs[kLeftOutput].setVoltage(left);
        }

        if (outputs[kRightOutput].isConnected()) {
            outputs[kRightOutput].setVoltage(right);
        }
    }
};

//--------------------------------------------------------------
// GAINWidget
//--------------------------------------------------------------

struct GAINWidget : ModuleWidget {
    GAINWidget(GAIN* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GAIN.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef GAIN_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        // track
        const int faderXofs = 6;
        const int faderYofs = -10;
        const int meterH = 9;
        const int meterW = 144;

        addParam(createParam<MFader>(Vec(33 + faderXofs, 46 + faderYofs), module, GAIN::kFader));

        VUMeter* meter = new VUMeter();
        if (module) {
            meter->vuLevels = &(module->vuLevels);
        }
        meter->box.pos = Vec(33, 46);
        meter->box.size = Vec(meterH, meterW);
        addChild(meter);

        // mute and level
        addParam(createParamCentered<MToggleButton>(Vec(37.5, 217), module, GAIN::kMute));
        addInput(createInputCentered<PJ301MPort>(Vec(37.5, 254), module, GAIN::kLevelInput));

        // ins and outs
        addInput(createInputCentered<PJ301MPort>(Vec(19.5, 292), module, GAIN::kLeftInput));
        addInput(createInputCentered<PJ301MPort>(Vec(55.5, 292), module, GAIN::kRightInput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(19.5, 334), module, GAIN::kLeftOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(55.5, 334), module, GAIN::kRightOutput));
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
