#include "arc_dsp.hpp"
#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct CLIP : Module {

    const int kOversampleFactor = 4;
    arc::dsp::Oversample oversample{kOversampleFactor};

    VuStats vuStats;
    Amplitude levelAmp;
    // Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

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

        configParam<LevelParamQuantity>(kLevelParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configParam(kLevelCvAmountParam, -1.0f, 1.0f, 0.0f, "Level CV amount");
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
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        vuStats.onSampleRateChange(e.sampleRate);
        levelAmp.onSampleRateChange(e.sampleRate);
    }

    float oversampleSoftClip(float in) {

        float buffer[arc::dsp::kMaxOversample] = {};
        oversample.upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = arc::dsp::softClip(buffer[i]);
        }

        return oversample.downsample(buffer);
    }

    void process(const ProcessArgs& args) override {

        if (!outputs[kOutput].isConnected()) {
            vuStats.process(0.0f);
            return;
        }

        // level amplitude
        float db = levelToDb(params[kLevelParam].getValue());
        float ampL = levelAmp.next(db);

        // process each channel
        float sum = 0;
        int channels = std::max(inputs[kInput].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = inputs[kInput].getPolyVoltage(ch);

            // channel amplitude
            float ampCh = ampL;
            // if (inputs[kLevelInput].isConnected()) {
            //     ampCh = ampCh * nextLevelAmplitude(ch);
            // }

            // process sample
            float limit = 5.0f * ampCh;
            float out = oversampleSoftClip(in / limit) * limit;

            sum += out;
            outputs[kOutput].setVoltage(out, ch);
        }
        outputs[kOutput].setChannels(channels);

        vuStats.process(sum);
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
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 12), module, CLIP::kDebug1));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 36), module, CLIP::kDebug2));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 60), module, CLIP::kDebug3));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 84), module, CLIP::kDebug4));
#endif

        addMeter(24 - 6, 44, module ? &(module->vuStats) : NULL);
        addMeter(24 + 1, 44, module ? &(module->vuStats) : NULL);

        addParam(createParamCentered<MKnob24>(Vec(22.5, 182), module, CLIP::kLevelParam));
        addParam(createParamCentered<MKnob18>(Vec(22.5, 218), module, CLIP::kLevelCvAmountParam));
        addInput(createInputCentered<MPolyPort>(Vec(22.5, 254), module, CLIP::kLevelCvInput));

        addInput(createInputCentered<MPolyPort>(Vec(22.5, 293), module, CLIP::kInput));
        addOutput(createOutputCentered<MPolyPort>(Vec(22.5, 334), module, CLIP::kOutput));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelCLIP = createModel<CLIP, CLIPWidget>("CLIP");
