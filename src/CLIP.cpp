#include <vector>

#include "arc_dsp.hpp"
#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct CLIP : Module {

    static constexpr float kRampTime = 0.010f;
    arc::dsp::LinearRamp levelRamp;
    arc::dsp::LinearRamp levelCvRamps[engine::PORT_MAX_CHANNELS];

    static const int kOversampleFactor = 4;
    std::vector<arc::dsp::Oversample> oversample;

    enum ParamId { kLevelParam, kParamsLen };

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

        configParam<LevelParamQuantity>(kLevelParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configInput(kLevelCvInput, "Level CV");

        configInput(kInput, "Audio");
        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef CLIP_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif

        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            oversample.push_back(arc::dsp::Oversample(kOversampleFactor));
        }
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {

        levelRamp.onSampleRateChange(e.sampleRate);
        levelRamp.setTime(kRampTime);
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelCvRamps[ch].onSampleRateChange(e.sampleRate);
            levelCvRamps[ch].setTime(kRampTime);
        }

        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            oversample[ch].onSampleRateChange(e.sampleRate);
        }
    }

    float nextLevelCvAmp(int ch) {
        float v = inputs[kLevelCvInput].getPolyVoltage(ch);
        float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        return arc::dsp::decibelsToAmplitude(levelCvRamps[ch].next(db));
    }

    float processChannel(int ch, float in, float limit) {

        float buffer[arc::dsp::kMaxOversample] = {};
        oversample[ch].upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = arc::dsp::softClip(buffer[i] / limit) * limit;
        }

        return oversample[ch].downsample(buffer);
    }

    void process(const ProcessArgs& args) override {
        if (!outputs[kOutput].isConnected()) {
            return;
        }

        float db = levelToDb(params[kLevelParam].getValue());
        float amp = arc::dsp::decibelsToAmplitude(levelRamp.next(db));

        int channels = std::max(inputs[kInput].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = inputs[kInput].getPolyVoltage(ch);

            float chAmp = amp;
            if (inputs[kLevelCvInput].isConnected()) {
                chAmp = chAmp * nextLevelCvAmp(ch);
            }

            float out = processChannel(ch, in, 5.0f * chAmp);
            outputs[kOutput].setVoltage(out, ch);
        }
        outputs[kOutput].setChannels(channels);
    }
};

//--------------------------------------------------------------
// CLIPWidget
//--------------------------------------------------------------

struct CLIPWidget : ModuleWidget {

    CLIPWidget(CLIP* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/CLIP.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef CLIP_DEBUG
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 12), module, CLIP::kDebug1));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 36), module, CLIP::kDebug2));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 60), module, CLIP::kDebug3));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 84), module, CLIP::kDebug4));
#endif

        addParam(createParamCentered<ArcKnob24>(Vec(22.5, 74), module, CLIP::kLevelParam));
        addInput(createInputCentered<ArcPolyPort>(Vec(22.5, 110), module, CLIP::kLevelCvInput));

        addInput(createInputCentered<ArcPolyPort>(Vec(22.5, 293), module, CLIP::kInput));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(22.5, 334), module, CLIP::kOutput));
    }
};

Model* modelCLIP = createModel<CLIP, CLIPWidget>("CLIP");
