#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define MIX2_DEBUG

//--------------------------------------------------------------
// MIX2
//--------------------------------------------------------------

struct MIX2 : Module {

    static const int kNumTracks = 2;

    // Amplitude faderAmp;
    // Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

    //// Since VUMeter monitors the levels in stereo, we will use StereoLevels,
    //// even though MIX2 is a mono Module.
    // StereoLevels levels;

#ifdef MIX2_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {

        // kFader1,
        // kFader2,
        // kFaderMix,

        kMute1,
        kMute2,
        kMuteMix,

        kPan1,
        kPan2,

        kParamsLen
    };

    enum InputId {

        kLeftInput1,
        kLeftInput2,

        kRightInput1,
        kRightInput2,

        kLevelInput1,
        kLevelInput2,
        kLevelInputMix,

        kPanInput1,
        kPanInput2,

        kInputsLen
    };

    enum OutputId {

        kLeftOutput,
        kRightOutput,

#ifdef MIX2_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    MIX2() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        // configParam<FaderParamQuantity>(kFader1, 0.0f, 1.0f, kFaderDbZero, "Track 1 Fader", "
        // dB"); configParam<FaderParamQuantity>(kFader2, 0.0f, 1.0f, kFaderDbZero, "Track 2 Fader",
        // " dB"); configParam<FaderParamQuantity>(kFaderMix, 0.0f, 1.0f, kFaderDbZero, "Mix Fader",
        // " dB");

        configSwitch(kMute1, 0.f, 1.f, 0.f, "Track 1 Mute", {"Off", "On"});
        configSwitch(kMute2, 0.f, 1.f, 0.f, "Track 2 Mute", {"Off", "On"});
        configSwitch(kMuteMix, 0.f, 1.f, 0.f, "Mix Mute", {"Off", "On"});

        configInput(kLevelInput1, "Track 1 Level CV");
        configInput(kLevelInput2, "Track 2 Level CV");
        configInput(kLevelInputMix, "Mix Level CV");

        configParam(kPan1, -1.0f, 1.0f, 0.0f, "Track 1 Pan");
        configParam(kPan2, -1.0f, 1.0f, 0.0f, "Track 2 Pan");

        configInput(kPanInput1, "Track 1 Pan CV");
        configInput(kPanInput2, "Track 2 Pan CV");

        configInput(kLeftInput1, "Track 1 Left");
        configInput(kLeftInput2, "Track 2 Left");

        configInput(kRightInput1, "Track 1 Right");
        configInput(kRightInput2, "Track 2 Right");

        configOutput(kLeftOutput, "Mix Left");
        configOutput(kRightOutput, "Mix Right");

#ifdef MIX2_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    // void onSampleRateChange(const SampleRateChangeEvent& e) override {
    //     faderAmp.sampleRateChange(e.sampleRate);
    //     for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
    //         levelAmps[ch].sampleRateChange(e.sampleRate);
    //     }
    //     levels.sampleRateChange(e.sampleRate);
    // }

    // float nextLevelAmplitude(int ch) {
    //     float lv = inputs[kLevelInput].getPolyVoltage(ch);
    //     // Scale the level input voltage exponentially from [0V, 10V] to [-72dB, +6dB].
    //     float db = rescale(lv, 0.0f, 10.0f, kMinDb, kMaxDb);
    //     return levelAmps[ch].next(db);
    // }

    void process(const ProcessArgs& args) override {
    }
};

//--------------------------------------------------------------
// MIX2Widget
//--------------------------------------------------------------

struct MIX2Widget : ModuleWidget {
    MIX2Widget(MIX2* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MIX2.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef MIX2_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, MIX2::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, MIX2::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, MIX2::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, MIX2::kDebug4));
#endif

        //////////////////////////////////////////
        //// fader and meter

        // const float faderXofs = 8;
        // const float faderYofs = -9.5;
        // const float meterH = 9;
        // const float meterW = 144;

        //// fader
        // addParam(createParam<RmFader>(Vec(21 + faderXofs, 46 + faderYofs), module,
        // MIX2::kFader));

        //// meter
        // VUMeter* meter = new VUMeter();
        // if (module) {
        //     meter->levels = &(module->levels);
        // }
        // meter->box.pos = Vec(21, 46);
        // meter->box.size = Vec(meterH, meterW);
        // addChild(meter);

        //////////////////////////////////////////
        //// mute and level

        // addParam(createParamCentered<RmToggleButton>(Vec(22.5, 217), module, MIX2::kMute));
        // addInput(createInputCentered<PJ301MPort>(Vec(22.5, 254), module, MIX2::kLevelInput));

        //////////////////////////////////////////
        //// ins and outs

        // addInput(createInputCentered<PJ301MPort>(Vec(22.5, 292), module, MIX2::kInput));
        // addOutput(createOutputCentered<PJ301MPort>(Vec(22.5, 334), module, MIX2::kOutput));

        // Columns: [28, 70, 112]
        // Rows: [203, 232, 261, 290, 319, 348]

        static const int cols[] = {28, 70};
        static const int mixCol = 112;

        for (int t = 0; t < MIX2::kNumTracks; t++) {
            addParam(createParamCentered<RmToggleButton>(Vec(cols[t], 203), module, MIX2::kMute1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 232), module, MIX2::kLevelInput1 + t));
            addParam(createParamCentered<RmKnob18>(Vec(cols[t], 261), module, MIX2::kPan1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 290), module, MIX2::kPanInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 319), module, MIX2::kLeftInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 348), module, MIX2::kRightInput1 + t));
        }

        addParam(createParamCentered<RmToggleButton>(Vec(mixCol, 203), module, MIX2::kMuteMix));
        addInput(createInputCentered<PJ301MPort>(Vec(mixCol, 232), module, MIX2::kLevelInputMix));
        // addInput(createInputCentered<PJ301MPort>(Vec(mixCol, 319), module, MIX2::kLeftOutput));
        // addInput(createInputCentered<PJ301MPort>(Vec(mixCol, 348), module, MIX2::kRightOutput));

        // configSwitch(kMute1, 0.f, 1.f, 0.f, "Track 1 Mute", {"Off", "On"});
        // configInput(kLevelInput1, "Track 1 Level CV");
        // configParam(kPan1, -1.0f, 1.0f, 0.0f, "Track 1 Pan");
        // configInput(kPanInput1, "Track 1 Pan CV");
        // configInput(kLeftInput1, "Track 1 Left");
        // configInput(kRightInput1, "Track 1 Right");

        // configSwitch(kMuteMix, 0.f, 1.f, 0.f, "Mix Mute", {"Off", "On"});
        // configInput(kLevelInputMix, "Mix Level CV");
        // configOutput(kLeftOutput, "Mix Left");
        // configOutput(kRightOutput, "Mix Right");
    }
};

Model* modelMIX2 = createModel<MIX2, MIX2Widget>("MIX2");
