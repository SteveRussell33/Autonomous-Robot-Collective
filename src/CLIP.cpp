#include "arc_dsp.hpp"
#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

#define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct SoftClip {
    const int kOversampleFactor = 4;
    arc::dsp::Oversample oversample{kOversampleFactor};

    void onSampleRateChange(float sampleRate) {
        oversample.onSampleRateChange(sampleRate);
    }

    float clip(float in) {

        //return arc::dsp::softClip(in);

        float buffer[arc::dsp::kMaxOversample] = {};
        oversample.upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = arc::dsp::softClip(buffer[i]);
        }

        return oversample.downsample(buffer);
    }
};

struct CLIP : Module {

    Amplitude levelAmp;
    //Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];
    SoftClip softClips[engine::PORT_MAX_CHANNELS];
    VuStats vuStats;

#ifdef CLIP_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

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
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {

        levelAmp.onSampleRateChange(e.sampleRate);

        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            //levelCvAmps[ch].onSampleRateChange(e.sampleRate);
            softClips[ch].onSampleRateChange(e.sampleRate);
        }

        vuStats.onSampleRateChange(e.sampleRate);
    }

    //float nextLevelCvAmp( int ch) {
    //    float v = inputs[kLevelCvInput].getPolyVoltage(ch);
    //    float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
    //    return levelCvAmps[ch].next(db);
    //}

    void process(const ProcessArgs& args) override {

        if (!outputs[kOutput].isConnected()) {
            vuStats.process(0.0f);
            return;
        }

        // level amplitude
        float db = params[kLevelParam].getValue();
        float amp = levelAmp.next(levelToDb(db));

        // process each channel
        float sum = 0;
        int channels = std::max(inputs[kInput].getChannels(), 1);

        for (int ch = 0; ch < channels; ch++) {
            float in = inputs[kInput].getPolyVoltage(ch);

            // channel amplitude
            float chAmp = amp;
            //if (inputs[kLevelCvInput].isConnected()) {
            //    chAmp = chAmp * nextLevelCvAmp(ch);
            //}

            // process sample
            float limit = 5.0f * chAmp;
            float out = softClips[ch].clip(in / limit) * limit;

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

        addParam(createParamCentered<MKnob24>(Vec(22.5, 186), module, CLIP::kLevelParam));
        addInput(createInputCentered<MPolyPort>(Vec(22.5, 222), module, CLIP::kLevelCvInput));

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
