#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct BoostParamQuantity : ParamQuantity {

    float getDisplayValue() override {
        float v = getValue();
        if (!module) {
            return v;
        }
        return rescale(v, 0.0f, 4.0f, -24.0f, 24.0f);
    }

    void setDisplayValue(float v) override {
        if (!module) {
            return;
        }
        v = clamp(v, -24.0f, 24.0f);
        setValue(rescale(v, -24.0f, 24.0f, 0.0f, 4.0f));
    }
};

struct GAIN : Module {

    Amplitude levelAmp;
    Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];

    VuStats vuStats;

    enum ParamId { kLevelParam, kMuteParam, kBoostParam, kParamsLen };

    enum InputId { kInput, kLevelCvInput, kInputsLen };

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

        configParam<LevelParamQuantity>(kLevelParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configInput(kLevelCvInput, "Level CV");
        configSwitch(kMuteParam, 0.f, 1.f, 0.f, "Mute", {"Off", "On"});

        configParam<BoostParamQuantity>(kBoostParam, 0.0f, 4.0f, 2.0f, "Boost/Cut", " dB");
        getParamQuantity(kBoostParam)->snapEnabled = true;

        configInput(kInput, "Audio");
        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef GAIN_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {

        levelAmp.onSampleRateChange(e.sampleRate);
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelCvAmps[ch].onSampleRateChange(e.sampleRate);
        }

        vuStats.onSampleRateChange(e.sampleRate);
    }

    float nextLevelCvAmp(int ch) {
        float v = inputs[kLevelCvInput].getPolyVoltage(ch);
        float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        return arc::dsp::decibelsToAmplitude(db);
    }

    void process(const ProcessArgs& args) override {

        // Check if anything is connected
        if (!inputs[kInput].isConnected() && !outputs[kOutput].isConnected()) {
            vuStats.process(args.sampleTime, 0.0f);
            return;
        }

        float sum = 0.0f;
        int channels = std::max(inputs[kInput].getChannels(), 1);

        // Muted
        if (params[kMuteParam].getValue() > 0.5f) {

            for (int ch = 0; ch < channels; ch++) {
                float m = levelCvAmps[ch].nextMute();

                float out = clamp(inputs[kInput].getPolyVoltage(ch) * m, -10.0f, 10.0f);
                if (outputs[kOutput].isConnected()) {
                    outputs[kOutput].voltages[ch] = out;
                }
                sum += out;
            }
        }
        // process normally
        else {
            float db = levelToDb(params[kLevelParam].getValue());
            db += rescale(params[kBoostParam].getValue(), 0.0f, 4.0f, -24.0f, 24.0f);
            float amp = levelAmp.next(db);

            for (int ch = 0; ch < channels; ch++) {
                float chAmp = amp;

                if (inputs[kLevelCvInput].isConnected()) {
                    chAmp = chAmp * nextLevelCvAmp(ch);
                }

                float out = clamp(inputs[kInput].getPolyVoltage(ch) * chAmp, -10.0f, 10.0f);
                if (outputs[kOutput].isConnected()) {
                    outputs[kOutput].voltages[ch] = out;
                }
                sum += out;
            }
        }

        if (outputs[kOutput].isConnected()) {
            outputs[kOutput].channels = channels;
        }

        vuStats.process(args.sampleTime, sum * 0.2f);
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
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        addMeter(24 - 6, 44, module ? &(module->vuStats) : NULL);
        addMeter(24 + 1, 44, module ? &(module->vuStats) : NULL);

        addParam(createParamCentered<ArcKnob24>(Vec(22.5, 176), module, GAIN::kLevelParam));
        addInput(createInputCentered<ArcPolyPort>(Vec(22.5, 206), module, GAIN::kLevelCvInput));
        addParam(createParamCentered<ArcMuteButton>(Vec(22.5, 236), module, GAIN::kMuteParam));

        auto w = createParamCentered<ArcKnob18>(Vec(22.5, 262), module, GAIN::kBoostParam);
        auto k = dynamic_cast<SvgKnob*>(w);
        k->minAngle = -M_PI / 2.0f;
        k->maxAngle = M_PI / 2.0f;
        addParam(w);

        addInput(createInputCentered<ArcPolyPort>(Vec(22.5, 293), module, GAIN::kInput));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(22.5, 334), module, GAIN::kOutput));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
