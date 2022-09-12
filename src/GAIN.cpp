#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    Amplitude faderAmp;
    Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

    // Since VUMeter monitors the levels in stereo, we will use StereoLevels,
    // even though GAIN is a mono Module.
    StereoLevels levels;

#ifdef GAIN_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {
        kFader,
        kMute,

        kParamsLen
    };

    enum InputId {
        kLevelInput,
        kInput,

        kInputsLen
    };

    enum OutputId {
        kOutput,

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

        configParam<FaderParamQuantity>(kFader, 0.0f, 1.0f, kFaderDbZero, "Fader", " dB");
        configSwitch(kMute, 0.f, 1.f, 0.f, "Mute", {"Off", "On"});

        configInput(kLevelInput, "Level");
        configInput(kInput, "Signal");

        configOutput(kOutput, "Signal");

        configBypass(kInput, kOutput);

#ifdef GAIN_DEBUG
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

    float nextLevelAmplitude(int ch) {
        float lv = inputs[kLevelInput].getPolyVoltage(ch);
        // Scale the level input voltage exponentially from [0V, 10V] to [-72dB, +6dB].
        float db = rescale(lv, 0.0f, 10.0f, kMinDb, kMaxDb);
        return levelAmps[ch].next(db);
    }

    void process(const ProcessArgs& args) override {

        // if the output isn't connected, just show the VU metering
        if (!outputs[kOutput].isConnected()) {
            float sum = 0;
            int channels = std::max(inputs[kInput].getChannels(), 1);
            for (int ch = 0; ch < channels; ch++) {
                sum += inputs[kInput].getPolyVoltage(ch);
            }
            levels.left.process(sum);
            levels.right.process(sum);
            return;
        }

        bool muted = params[kMute].getValue() > 0.5f;

        // fader amplitude
        float ampF = 0.0f;
        if (muted) {
            ampF = faderAmp.next(kMinDb);
        } else {
            float db = faderToDb(params[kFader].getValue());
            ampF = faderAmp.next(db);
        }

        // process each channel
        float sum = 0;
        int channels = std::max(inputs[kInput].getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = inputs[kInput].getPolyVoltage(ch);

            // channel amplitude
            float ampCh = ampF;
            if (!muted && inputs[kLevelInput].isConnected()) {
                ampCh = ampCh * nextLevelAmplitude(ch);
            }

            // process sample
            float out = in * ampCh;

            sum += out;
            outputs[kOutput].setVoltage(out, ch);
        }
        outputs[kOutput].setChannels(channels);

        levels.left.process(sum);
        levels.right.process(sum);
    }
};

//--------------------------------------------------------------
// GAINWidget
//--------------------------------------------------------------

struct GAINWidget : ModuleWidget {
    GAINWidget(GAIN* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GAIN.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef GAIN_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        ////////////////////////////////////////
        // fader and meter

        const float faderXofs = 8;
        const float faderYofs = -9.5;
        const float meterH = 9;
        const float meterW = 144;

        // fader
        addParam(createParam<RmFader>(Vec(21 + faderXofs, 46 + faderYofs), module, GAIN::kFader));

        // meter
        VUMeter* meter = new VUMeter();
        if (module) {
            meter->levels = &(module->levels);
        }
        meter->box.pos = Vec(21, 46);
        meter->box.size = Vec(meterH, meterW);
        addChild(meter);

        ////////////////////////////////////////
        // mute and level

        addParam(createParamCentered<RmToggleButton>(Vec(22.5, 217), module, GAIN::kMute));
        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 254), module, GAIN::kLevelInput));

        ////////////////////////////////////////
        // ins and outs

        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 292), module, GAIN::kInput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(22.5, 334), module, GAIN::kOutput));
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
