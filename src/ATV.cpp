
#include "bogaudio/dsp/signal.hpp"

#include "plugin.hpp"
#include "widgets.hpp"

// define ATV_DEBUG

struct ATV : Module {

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

    bogaudio::dsp::SlewLimiter paramSlew[kParamsLen];

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

    void applyCV(int inputID, int paramID, int outputID) {

        if (!outputs[outputID].isConnected()) {
            return;
        }

        float pv = slewParam(paramID);

        int channels = std::max(inputs[inputID].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {

            float in = inputs[inputID].getPolyVoltage(ch);
            outputs[outputID].setVoltage(in * pv, ch);
        }

        outputs[outputID].setChannels(channels);
    }

    inline float slewParam(int id) {
        float v = params[id].getValue();
        return paramSlew[id].next(v);
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        paramSlew[kCvParamA].setParams(e.sampleRate, 5.0f, 2.0f);
        paramSlew[kCvParamB].setParams(e.sampleRate, 5.0f, 2.0f);
    }

    void process(const ProcessArgs& args) override {
        applyCV(kInputA, kCvParamA, kOutputA);
        applyCV(kInputB, kCvParamB, kOutputB);
    }
};

struct ATVWidget : ModuleWidget {
    ATVWidget(ATV* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/ATV.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef ATV_DEBUG
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 12), module, ATV::kDebug1));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 36), module, ATV::kDebug2));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 60), module, ATV::kDebug3));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 84), module, ATV::kDebug4));
#endif

        addInput(createInputCentered<ArcPolyPort>(Vec(15, 74), module, ATV::kInputA));
        addParam(createParamCentered<ArcKnob18>(Vec(15, 116), module, ATV::kCvParamA));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(15, 158), module, ATV::kOutputA));

        addInput(createInputCentered<ArcPolyPort>(Vec(15, 224), module, ATV::kInputB));
        addParam(createParamCentered<ArcKnob18>(Vec(15, 266), module, ATV::kCvParamB));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(15, 308), module, ATV::kOutputB));
    }
};

Model* modelATV = createModel<ATV, ATVWidget>("ATV");
