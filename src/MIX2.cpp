#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define MIX2_DEBUG

//--------------------------------------------------------------
// MIX2
//--------------------------------------------------------------

struct MIX2 : Module {

    static const int kNumTracks = 2;

    StereoTrack tracks[kNumTracks];

#ifdef MIX2_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {

        kVolumeParam1,
        kVolumeParam2,
        kVolumeParamMix,

        kMuteParam1,
        kMuteParam2,
        kMuteParamMix,

        kPanParam1,
        kPanParam2,

        kParamsLen
    };

    enum InputId {

        kLeftInput1,
        kLeftInput2,

        kRightInput1,
        kRightInput2,

        kVolumeInput1,
        kVolumeInput2,
        kVolumeInputMix,

        kPanParamInput1,
        kPanParamInput2,

        kInputsLen
    };

    enum OutputId {

        kSendLeftOutput,
        kSendRightOutput,
        kMixLeftOutput,
        kMixRightOutput,

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

        // configParam<FaderParamQuantity>(kVolumeParam1, 0.0f, 1.0f, kVolumeParamDbZero, "Fader", " dB");
        // configParam<FaderParamQuantity>(kVolumeParam2, 0.0f, 1.0f, kVolumeParamDbZero, "Fader", " dB");
        // configParam<FaderParamQuantity>(kVolumeParamMix, 0.0f, 1.0f, kVolumeParamDbZero, "Fader", " dB");

        configSwitch(kMuteParam1, 0.f, 1.f, 0.f, "Track 1 Mute", {"Off", "On"});
        configSwitch(kMuteParam2, 0.f, 1.f, 0.f, "Track 2 Mute", {"Off", "On"});
        configSwitch(kMuteParamMix, 0.f, 1.f, 0.f, "Mix Mute", {"Off", "On"});

        configInput(kVolumeInput1, "Track 1 Volume CV");
        configInput(kVolumeInput2, "Track 2 Volume CV");
        configInput(kVolumeInputMix, "Mix Volume CV");

        configParam(kPanParam1, -1.0f, 1.0f, 0.0f, "Track 1 Pan");
        configParam(kPanParam2, -1.0f, 1.0f, 0.0f, "Track 2 Pan");

        configInput(kPanParamInput1, "Track 1 Pan CV");
        configInput(kPanParamInput2, "Track 2 Pan CV");

        configInput(kLeftInput1, "Track 1 Left");
        configInput(kLeftInput2, "Track 2 Left");

        configInput(kRightInput1, "Track 1 Right");
        configInput(kRightInput2, "Track 2 Right");

        configOutput(kSendLeftOutput, "Send Left");
        configOutput(kSendRightOutput, "Send Right");

        configOutput(kMixLeftOutput, "Mix Left");
        configOutput(kMixRightOutput, "Mix Right");

#ifdef MIX2_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        for (int t = 0; t < kNumTracks; t++) {
            tracks[t].onSampleRateChange(e.sampleRate);
        }
    }

    void process(const ProcessArgs& args) override {

        // Process each track
        for (int t = 0; t < kNumTracks; t++) {
            tracks[t].process(inputs[kLeftInput1 + t], inputs[kRightInput1 + t]);
        }
    }
};

//--------------------------------------------------------------
// MIX2Widget
//--------------------------------------------------------------

struct MIX2Widget : ModuleWidget {

    MIX2Widget(MIX2* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MIX2.svg")));

        addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

#ifdef MIX2_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, MIX2::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, MIX2::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, MIX2::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, MIX2::kDebug4));
#endif

        //////////////////////////////////////////////////////////////

        static const int cols[] = {31, 72};
        static const int mixCol = 113;

        // [174, 203, 232, 261, 290, 319, 348]

        for (int t = 0; t < MIX2::kNumTracks; t++) {
            addParam(createParamCentered<RmKnob24>(Vec(cols[t], 174), module, MIX2::kVolumeParam1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 203), module, MIX2::kVolumeInput1 + t));
            addParam(createParamCentered<RmToggleButton>(Vec(cols[t], 232), module, MIX2::kMuteParam1 + t));
            addParam(createParamCentered<RmKnob24>(Vec(cols[t], 261), module, MIX2::kPanParam1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 290), module, MIX2::kPanParamInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 319), module, MIX2::kLeftInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 348), module, MIX2::kRightInput1 + t));
        }

        addParam(createParamCentered<RmKnob24>(Vec(mixCol, 174), module, MIX2::kVolumeParamMix));
        addInput(createInputCentered<PJ301MPort>(Vec(mixCol, 203), module, MIX2::kVolumeInputMix));
        addParam(createParamCentered<RmToggleButton>(Vec(mixCol, 232), module, MIX2::kMuteParamMix));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 261), module, MIX2::kSendLeftOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 290), module, MIX2::kSendRightOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 319), module, MIX2::kMixLeftOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 348), module, MIX2::kMixRightOutput));
    }
};

Model* modelMIX2 = createModel<MIX2, MIX2Widget>("MIX2");
