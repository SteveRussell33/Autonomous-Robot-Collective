#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    Amplitude levelAmp;
    Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];

    VuStats vuStats;

    enum ParamId { kLevelParam, kMuteParam, kParamsLen };

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

    void process(const ProcessArgs& args) override {

        if (outputs[kOutput].isConnected()) {

            float sum = 0.0f;
            int channels = std::max(inputs[kInput].getChannels(), 1);

            if (params[kMuteParam].getValue() > 0.5f) {

                for (int ch = 0; ch < channels; ch++) {
                    float m = levelCvAmps[ch].nextMute();

                    float out = clamp(inputs[kInput].getPolyVoltage(ch) * m, -10.0f, 10.0f);
                    outputs[kOutput].voltages[ch] = out;
                    sum += out;
                }
            } else {

                float amp = levelAmp.next(levelToDb(params[kLevelParam].getValue()));

                for (int ch = 0; ch < channels; ch++) {
                    float chAmp = amp;

                    if (inputs[kLevelCvInput].isConnected()) {
                        float lv = inputs[kLevelCvInput].getPolyVoltage(ch);
                        float db = kMinDb + lv * 0.1f * kDecibelRange;
                        float nl = levelCvAmps[ch].next(db);
                        chAmp *= nl;
                    }

                    float out = clamp(inputs[kInput].getPolyVoltage(ch) * chAmp, -10.0f, 10.0f);
                    outputs[kOutput].voltages[ch] = out;
                    sum += out;
                }
            }
            outputs[kOutput].channels = channels;

            vuStats.process(args.sampleTime, sum * 0.2f);
        } 
        // if the output is not connected, just meter the input
        else {
            if (inputs[kInput].isConnected()) {
                vuStats.process(args.sampleTime, inputs[kInput].getVoltageSum() * 0.2f);
            } else {
                vuStats.process(args.sampleTime, 0.0f);
            }
        }
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
