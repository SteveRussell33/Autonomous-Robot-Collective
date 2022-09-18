#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct CLIP : Module {

#ifdef CLIP_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId { kLevelParam, kLevelCvAmountParam, kParamsLen };

    enum InputId { kInput, kLevelCvInput, kInputsLen };

    enum OutputId {
        kOutput,

#ifdef CLIP_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    CLIP() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam<FaderParamQuantity>(kLevelParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configParam(kLevelCvAmountParam, -1.0f, 1.0f, 0.0f, "Level CV amount");
        configInput(kLevelCvInput, "Level CV");

        configInput(kInput, "Audio");
        configOutput(kOutput, "Audio");

#ifdef CLIP_DEBUG
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
// CLIPWidget
//--------------------------------------------------------------

struct CLIPWidget : ModuleWidget {

    CLIPWidget(CLIP* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/CLIP.svg")));

        // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef CLIP_DEBUG
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 12), module, CLIP::kDebug1));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 36), module, CLIP::kDebug2));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 60), module, CLIP::kDebug3));
        addOutput(createOutputCentered<RmPolyPort>(Vec(12, 84), module, CLIP::kDebug4));
#endif

        //addMeter(24 - 6, 44, module ? &(module->track.left.vuLevel) : NULL);
        //addMeter(24 + 1, 44, module ? &(module->track.right.vuLevel) : NULL);

        addParam(createParamCentered<RmKnob24>(Vec(22.5, 188), module, CLIP::kLevelParam));
        addParam(createParamCentered<RmKnob18>(Vec(22.5, 224), module, CLIP::kLevelCvAmountParam));
        addInput(createInputCentered<RmPolyPort>(Vec(22.5, 260), module, CLIP::kLevelCvInput));

        addInput(createInputCentered<RmPolyPort>(Vec(22.5, 293), module, CLIP::kInput));
        addOutput(createOutputCentered<RmPolyPort>(Vec(22.5, 334), module, CLIP::kOutput));
    }

    //void addMeter(float x, float y, VuLevel* vuLevel) {
    //    VuMeter* meter = new VuMeter(vuLevel);
    //    meter->box.pos = Vec(x, y);
    //    meter->box.size = Vec(8, 104);
    //    addChild(meter);
    //}
};

Model* modelCLIP = createModel<CLIP, CLIPWidget>("CLIP");
