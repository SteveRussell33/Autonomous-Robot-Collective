#include "plugin.hpp"
#include "widgets.hpp"

#include "bogaudio/dsp/pitch.hpp"
#include "bogaudio/dsp/signal.hpp"

// define FM_DEBUG

struct FM : Module {

    enum ParamId {
        kRatioParam,
        kRatioCvAmountParam,
        kRatioQuantParam,
        kOffsetParam,
        kOffsetCvAmountParam,

        kParamsLen
    };

    enum InputId {
        kRatioCvInput,
        kOffsetCvInput,
        kCarrierPitchInput,

        kInputsLen
    };

    enum OutputId {
        kModulatorPitchOutput,

#ifdef FM_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    bogaudio::dsp::SlewLimiter paramSlew[kParamsLen];

    FM() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam(kRatioParam, 0.01f, 10.0f, 1.0f, "Ratio");
        configParam(kRatioCvAmountParam, -1.0f, 1.0f, 0.0f, "Ratio CV amount");
        configSwitch(kRatioQuantParam, 0.f, 1.f, 0.f, "Quantize Ratio", {"On", "Off"});

        configParam(kOffsetParam, -5.0f, 5.0f, 0.0f, "Offset", " Hz", 0.0f, 40.0f);
        configParam(kOffsetCvAmountParam, -1.0f, 1.0f, 0.0f, "Offset CV amount");

        configInput(kRatioCvInput, "Ratio CV");
        configInput(kOffsetCvInput, "Offset CV");
        configInput(kCarrierPitchInput, "Carrier V/Oct");

        configOutput(kModulatorPitchOutput, "Modulator V/Oct");

        configBypass(kCarrierPitchInput, kModulatorPitchOutput);

#ifdef FM_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    float quantizeRatio(float ratio) {
        if (ratio < 0.1875f)
            return 0.125f;
        else if (ratio < 0.375f)
            return 0.25f;
        else if (ratio < 0.75f)
            return 0.5f;
        else
            return round(ratio);
    }

    inline float slewParam(int id) {
        float v = params[id].getValue();
        return paramSlew[id].next(v);
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        // clang-format off
        paramSlew[kRatioParam]         .setParams(e.sampleRate, 5.0f, 10.0f);
        paramSlew[kRatioCvAmountParam] .setParams(e.sampleRate, 5.0f,  2.0f);
        paramSlew[kOffsetParam]        .setParams(e.sampleRate, 5.0f, 10.0f);
        paramSlew[kOffsetCvAmountParam].setParams(e.sampleRate, 5.0f,  2.0f);
        // clang-format on
    }

    void process(const ProcessArgs& args) override {

        if (!outputs[kModulatorPitchOutput].isConnected()) {
            return;
        }

        float pRatio = slewParam(kRatioParam);
        float pRatioCvAmount = slewParam(kRatioCvAmountParam);
        float pOffset = slewParam(kOffsetParam);
        float pOffsetCvAmount = slewParam(kOffsetCvAmountParam);

        bool pRatioQuant = params[kRatioQuantParam].getValue() < 0.5f;

        int channels = std::max(inputs[kCarrierPitchInput].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {

            float inRatioCv = inputs[kRatioCvInput].getPolyVoltage(ch);
            float inOffsetCv = inputs[kOffsetCvInput].getPolyVoltage(ch);
            float inCarrierPitch = inputs[kCarrierPitchInput].getPolyVoltage(ch);

            // ratio
            float ratio = pRatio + inRatioCv * pRatioCvAmount;
            if (pRatioQuant) {
                ratio = quantizeRatio(ratio);
            }

            // offset
            float offset = pOffset + inOffsetCv * pOffsetCvAmount;
            offset = offset * 40.0f; // -200Hz to 200 Hz

            // frequency
            float carrierFreq = bogaudio::dsp::cvToFrequency(inCarrierPitch);
            float modulatorFreq = clamp(carrierFreq * ratio + offset, 20.0f, 20000.0f);
            float outModulatorPitch = bogaudio::dsp::frequencyToCV(modulatorFreq);

            outputs[kModulatorPitchOutput].setVoltage(outModulatorPitch, ch);
        }
        outputs[kModulatorPitchOutput].setChannels(channels);
    }
};

struct FMWidget : ModuleWidget {
    FMWidget(FM* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/FM.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // knobs and switches
        addParam(createParamCentered<MKnob45>(Vec(37.5, 90), module, FM::kRatioParam));
        addParam(createParamCentered<MHSwitch>(Vec(37.5, 132), module, FM::kRatioQuantParam));
        addParam(createParamCentered<MKnob45>(Vec(37.5, 197), module, FM::kOffsetParam));

        // row 1
        addParam(createParamCentered<MKnob18>(Vec(19.5, 250), module, FM::kRatioCvAmountParam));
        addParam(createParamCentered<MKnob18>(Vec(55.5, 250), module, FM::kOffsetCvAmountParam));

        // row 2
        addInput(createInputCentered<MPolyPort>(Vec(19.5, 292), module, FM::kRatioCvInput));
        addInput(createInputCentered<MPolyPort>(Vec(55.5, 292), module, FM::kOffsetCvInput));

        // row 3
        addInput(createInputCentered<MPolyPort>(Vec(19.5, 334), module, FM::kCarrierPitchInput));
        addOutput(
            createOutputCentered<MPolyPort>(Vec(55.5, 334), module, FM::kModulatorPitchOutput));

#ifdef FM_DEBUG
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 12), module, FM::kDebug1));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 36), module, FM::kDebug2));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 60), module, FM::kDebug3));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 84), module, FM::kDebug4));
#endif
    }
};

Model* modelFM = createModel<FM, FMWidget>("FM");
