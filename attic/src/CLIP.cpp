#include "plugin.hpp"
#include "rm_dsp.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct CLIP : Module {

    const int kOversampleFactor = 4;
    rm::dsp::Oversample oversample{kOversampleFactor};

    Amplitude faderAmp;
    Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

    // Since VUMeter monitors the levels in stereo, we will use StereoLevels,
    // even though CLIP is a mono Module.
    StereoLevels levels;

#ifdef CLIP_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {
        kFader,

        kParamsLen
    };

    enum InputId {
        kLevelInput,
        kInput,

        kInputsLen
    };

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

        configParam<FaderParamQuantity>(kFader, 0.0f, 1.0f, kFaderDbZero, "Fader", " dB");

        configInput(kLevelInput, "Level");
        configInput(kInput, "Signal");

        configOutput(kOutput, "Signal");

        configBypass(kInput, kOutput);

#ifdef CLIP_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        faderAmp.sampleRateChange(e.sampleRate);
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelAmps[ch].sampleRateChange(e.sampleRate);
        }
        levels.sampleRateChange(e.sampleRate);
    }

    float oversampleSoftClip(float in) {

        float buffer[rm::dsp::kMaxOversample] = {};
        oversample.upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = rm::dsp::softClip(buffer[i]);
        }

        return oversample.downsample(buffer);
    }

    // Scale the level input voltage exponentially from [0V, 10V] to [-72dB, +6dB].
    float nextLevelAmplitude(int ch) {
        float lv = inputs[kLevelInput].getPolyVoltage(ch);
        float db = rescale(lv, 0.0f, 10.0f, kMinDb, kMaxDb);
        return levelAmps[ch].next(db);
    }

    void process(const ProcessArgs& args) override {
        if (!outputs[kOutput].isConnected()) {
            levels.left.process(0.0f);
            levels.right.process(0.0f);
            return;
        }

        // fader amplitude
        float db = faderToDb(params[kFader].getValue());
        float ampF = faderAmp.next(db);

        // process each channel
        float sum = 0;
        int channels = std::max(inputs[kInput].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {

            // channel amplitude
            float ampCh = ampF;
            if (inputs[kLevelInput].isConnected()) {
                ampCh = ampCh * nextLevelAmplitude(ch);
            }

            // process sample
            float limit = 5.0f * ampCh;
            float in = inputs[kInput].getPolyVoltage(ch);
            float out = oversampleSoftClip(in / limit) * limit;

            sum += out;
            outputs[kOutput].setVoltage(out, ch);
        }
        outputs[kOutput].setChannels(channels);

        levels.left.process(sum);
        levels.right.process(sum);
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
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, CLIP::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, CLIP::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, CLIP::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, CLIP::kDebug4));
#endif

        addFader(22.5, 48, CLIP::kFader);
        addMeter(22.5, 48, module ? &(module->levels) : NULL);

        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 254), module, CLIP::kLevelInput));

        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 292), module, CLIP::kInput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(22.5, 334), module, CLIP::kOutput));
    }

    void addFader(float x, float y, int faderID) {
        static const float faderXofs = 5.5;
        static const float faderYofs = -9.5;

        addParam(createParam<RmFader>(Vec(x + faderXofs, y + faderYofs), module, faderID));
    }

    void addMeter(float x, float y, StereoLevels* levels) {
        static const float meterXofs = -1.5;
        static const float meterH = 9;
        static const float meterW = 144;

        VUMeter* meter = new VUMeter(levels);
        meter->box.pos = Vec(x + meterXofs, y);
        meter->box.size = Vec(meterH, meterW);
        addChild(meter);
    }
};

Model* modelCLIP = createModel<CLIP, CLIPWidget>("CLIP");