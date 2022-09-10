#include "mix.hpp"
#include "plugin.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    // This is a mono module, but its easier to do everything in stereo.
    StereoTrack track;

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
        track.sampleRateChange(e.sampleRate);
    }

    void process(const ProcessArgs& args) override {
        if (!inputs[kInput].isConnected()) {
            return;
        }

        float in = inputs[kInput].getVoltage();

        // This is a mono module, but its easier to do everything in stereo.
        track.left.process(in);
        track.right.process(in);

        if (outputs[kOutput].isConnected()) {
            outputs[kOutput].setVoltage(in);
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
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        // track
        const float faderXofs = 8;
        const float faderYofs = -9.5;
        const float meterH = 9;
        const float meterW = 144;

        addParam(createParam<MFader>(Vec(20 + faderXofs, 46 + faderYofs), module, GAIN::kFader));

        VUMeter* meter = new VUMeter();
        if (module) {
            meter->track = &(module->track);
        }
        meter->box.pos = Vec(20, 46);
        meter->box.size = Vec(meterH, meterW);
        addChild(meter);

        // mute and level
        addParam(createParamCentered<MToggleButton>(Vec(22.5, 217), module, GAIN::kMute));
        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 254), module, GAIN::kLevelInput));

        // ins and outs
        addInput(createInputCentered<PJ301MPort>(Vec(22.5, 292), module, GAIN::kInput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(22.5, 334), module, GAIN::kOutput));
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
