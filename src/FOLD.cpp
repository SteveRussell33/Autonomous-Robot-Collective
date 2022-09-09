
#include "oversample.hpp"
#include "plugin.hpp"
#include "widgets.hpp"

// define FOLD_DEBUG

struct FOLD : Module {

    const int kOversampleFactor = 4;
    Oversample oversample{kOversampleFactor};

#ifdef FOLD_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {
        kTimbreParam,
        kTimbreCvAmountParam,

        kParamsLen
    };

    enum InputId {
        kTimbreCvInput,
        kInput,

        kInputsLen
    };

    enum OutputId {
        kOutput,

#ifdef FOLD_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    FOLD() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam(kTimbreParam, 0.0f, 10.0f, 0.0f, "Timbre");
        configParam(kTimbreCvAmountParam, -1.0f, 1.0f, 0.0f, "Timbre CV amount");

        configInput(kTimbreCvInput, "Timbre CV");
        configInput(kInput, "Audio");

        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef FOLD_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        oversample.sampleRateChange(e.sampleRate);
    }

    // This wavefolding algorithm is derived from a Max/MSP patch that was
    // created by Randy Jones of Madrona Labs.
    inline float wavefold(float in, float timbre) {

        const float kTwoPi = 2.0f * M_PI;

        float ampOffset = timbre * 2.0f + 0.1f;
        float phaseOffset = timbre + 0.25f;

        float amp = in * ampOffset;

        // TODO should we use a fast approximation for cosf()?
        return std::cosf(kTwoPi * (amp + phaseOffset));
    }

    float oversampleFold(float in, float timbre) {

        float buffer[kMaxOversample] = {};
        oversample.upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = wavefold(buffer[i], timbre);
        }

        return oversample.downsample(buffer);
    }

    void process(const ProcessArgs& args) override {
        if (!outputs[kOutput].isConnected()) {
            return;
        }

        float pTimbre = params[kTimbreParam].getValue() / 10.0f;
        float pTimbreCvAmount = params[kTimbreCvAmountParam].getValue();

        int channels = std::max(inputs[kInput].getChannels(), 1);

        for (int ch = 0; ch < channels; ch++) {

            float inTimbreCv = inputs[kTimbreCvInput].getPolyVoltage(ch);
            float timbre = pTimbre + inTimbreCv * pTimbreCvAmount;
            // timbre = clamp(timbre, 0.0, 10.0);

            // Turn the timbre down a good bit so its less aggressive
            timbre = timbre * 0.5f;

            float in = inputs[kInput].getPolyVoltage(ch) / 5.0f;

            float out = oversampleFold(in, timbre);

            outputs[kOutput].setVoltage(out * 5.0f, ch);
        }

        outputs[kOutput].setChannels(channels);
    }
};

struct FOLDWidget : ModuleWidget {
    FOLDWidget(FOLD* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/FOLD.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef FOLD_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, FOLD::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, FOLD::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, FOLD::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, FOLD::kDebug4));
#endif

        addParam(createParamCentered<MKnob32>(Vec(22.5, 78), module, FOLD::kTimbreParam));
        addParam(createParamCentered<MKnob18>(Vec(22.5, 120), module, FOLD::kTimbreCvAmountParam));
        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 162), module, FOLD::kTimbreCvInput));

        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 292), module, FOLD::kInput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(22.5, 334), module, FOLD::kOutput));
    }
};

Model* modelFOLD = createModel<FOLD, FOLDWidget>("FOLD");
