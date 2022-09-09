
#include "plugin.hpp"
#include "widgets.hpp"

// define ATV_DEBUG

struct ATV : Module {

#ifdef ATV_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {
        kCvParamA,
        kCvParamB,

        kParamsLen
    };

    enum InputId {
        kInputA,
        kInputB,

        kInputsLen
    };

    enum OutputId {
        kOutputA,
        kOutputB,

#ifdef ATV_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    ATV() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam(kCvParamA, -1.0f, 1.0f, 0.0f, "A CV amount");
        configParam(kCvParamB, -1.0f, 1.0f, 0.0f, "B CV amount");

        configInput(kInputA, "A");
        configInput(kInputB, "B");

        configOutput(kOutputA, "A");
        configOutput(kOutputB, "B");

#ifdef ATV_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void process(const ProcessArgs& args) override {
    }
};

struct ATVWidget : ModuleWidget {
    ATVWidget(ATV* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/ATV.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef ATV_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, ATV::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, ATV::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, ATV::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, ATV::kDebug4));
#endif

        addInput(createInputCentered<PJ301MPort>(Vec(15, 74), module, ATV::kInputA));
        addParam(createParamCentered<MKnob18>(Vec(15, 116), module, ATV::kCvParamA));
        addOutput(createOutputCentered<PJ301MPort>(Vec(15, 158), module, ATV::kOutputA));

        addInput(createInputCentered<PJ301MPort>(Vec(15, 230), module, ATV::kInputB));
        addParam(createParamCentered<MKnob18>(Vec(15, 272), module, ATV::kCvParamB));
        addOutput(createOutputCentered<PJ301MPort>(Vec(15, 314), module, ATV::kOutputB));
    }
};

Model* modelATV = createModel<ATV, ATVWidget>("ATV");
