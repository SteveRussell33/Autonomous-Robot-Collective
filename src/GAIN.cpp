#include "plugin.hpp"
#include "widgets.hpp"
#include "track.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

#ifdef GAIN_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {
        kFaderParam,
        kMuteParam,
        kParamsLen
    };

    enum InputId {
        kLeftInput,
        kRightInput,
        kFaderCvInput,
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

        //configParam<FaderParamQuantity>(kFaderParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configSwitch(kMuteParam, 0.f, 1.f, 0.f, "Mute", {"Off", "On"});
        configInput(kFaderCvInput, "Track 1 Level CV");

        configInput(kLeftInput, "Left");
        configInput(kRightInput, "Right");

        configOutput(kLeftOutput, "Left");
        configOutput(kRightOutput, "Right");

#ifdef GAIN_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
    }

    void process(const ProcessArgs& args) override {
    }
};

//--------------------------------------------------------------
// GAINWidget
//--------------------------------------------------------------

struct GAINWidget : ModuleWidget {

    GAINWidget(GAIN* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GAIN.svg")));

        //addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        //addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        //addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        //addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef GAIN_DEBUG
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        //addMeter(26 - 6, 44, module ? &(module->tracks[t].left.vuLevel) : NULL);
        //addMeter(26 + 1, 44, module ? &(module->tracks[t].right.vuLevel) : NULL);
        addMeter(26 - 6, 44, NULL);
        addMeter(26 + 1, 44, NULL);

        addParam(createParamCentered<RmKnob24>(Vec(26, 174), module, GAIN::kFaderParam));
        addInput(createInputCentered<RmPolyPort>(Vec(26, 203), module, GAIN::kFaderCvInput));
        addParam(createParamCentered<RmToggleButton>(Vec(26, 232), module, GAIN::kMuteParam));

        addInput(createInputCentered<RmPolyPort>(Vec(26, 261), module, GAIN::kLeftInput));
        addInput(createInputCentered<RmPolyPort>(Vec(26, 290), module, GAIN::kRightInput));

        addOutput(createOutputCentered<RmMonoPort>(Vec(26, 319), module, GAIN::kLeftOutput));
        addOutput(createOutputCentered<RmMonoPort>(Vec(26, 348), module, GAIN::kRightOutput));
    }

    void addMeter(float x, float y, VuLevel* vuLevel) {
        VuMeter* meter = new VuMeter(vuLevel);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
