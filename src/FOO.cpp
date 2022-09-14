#include "plugin.hpp"
// include "track.hpp"
#include "widgets.hpp"

// define FOO_DEBUG

//--------------------------------------------------------------
// FOO
//--------------------------------------------------------------

struct FOO : Module {

    static const int kNumTracks = 2;

    // Amplitude faderAmp;
    // Amplitude levelAmps[engine::PORT_MAX_CHANNELS];
    // StereoLevels levels[kNumTracks+1];

#ifdef FOO_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId {

        kLevelParam1,
        kLevelParam2,
        kLevelParamMix,

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

        kLevelInput1,
        kLevelInput2,
        kLevelInputMix,

        kPanParamInput1,
        kPanParamInput2,

        kInputsLen
    };

    enum OutputId {

        kLeftOutput,
        kRightOutput,

#ifdef FOO_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    FOO() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        // configParam<FaderParamQuantity>(kLevelParam1, 0.0f, 1.0f, kLevelParamDbZero, "Fader", " dB");
        // configParam<FaderParamQuantity>(kLevelParam2, 0.0f, 1.0f, kLevelParamDbZero, "Fader", " dB");
        // configParam<FaderParamQuantity>(kLevelParamMix, 0.0f, 1.0f, kLevelParamDbZero, "Fader", " dB");

        configSwitch(kMuteParam1, 0.f, 1.f, 0.f, "Track 1 Mute", {"Off", "On"});
        configSwitch(kMuteParam2, 0.f, 1.f, 0.f, "Track 2 Mute", {"Off", "On"});
        configSwitch(kMuteParamMix, 0.f, 1.f, 0.f, "Mix Mute", {"Off", "On"});

        configInput(kLevelInput1, "Track 1 Level CV");
        configInput(kLevelInput2, "Track 2 Level CV");
        configInput(kLevelInputMix, "Mix Level CV");

        configParam(kPanParam1, -1.0f, 1.0f, 0.0f, "Track 1 Pan");
        configParam(kPanParam2, -1.0f, 1.0f, 0.0f, "Track 2 Pan");

        configInput(kPanParamInput1, "Track 1 Pan CV");
        configInput(kPanParamInput2, "Track 2 Pan CV");

        configInput(kLeftInput1, "Track 1 Left");
        configInput(kLeftInput2, "Track 2 Left");

        configInput(kRightInput1, "Track 1 Right");
        configInput(kRightInput2, "Track 2 Right");

        configOutput(kLeftOutput, "Mix Left");
        configOutput(kRightOutput, "Mix Right");

#ifdef FOO_DEBUG
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
// FOOWidget
//--------------------------------------------------------------

struct FOOWidget : ModuleWidget {

    FOOWidget(FOO* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/FOO.svg")));

        addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

#ifdef FOO_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, FOO::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, FOO::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, FOO::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, FOO::kDebug4));
#endif

        //////////////////////////////////////////////////////////////

        static const int cols[] = {31, 72};
        static const int mixCol = 113;

        // [168, 198, 228, 258, 288, 318, 348]

        for (int t = 0; t < FOO::kNumTracks; t++) {
            addParam(createParamCentered<RmKnob24>(Vec(cols[t], 168), module, FOO::kLevelParam1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 198), module, FOO::kLevelInput1 + t));
            addParam(createParamCentered<RmToggleButton>(Vec(cols[t], 228), module, FOO::kMuteParam1 + t));
            addParam(createParamCentered<RmKnob18>(Vec(cols[t], 258), module, FOO::kPanParam1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 288), module, FOO::kPanParamInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 318), module, FOO::kLeftInput1 + t));
            addInput(createInputCentered<PJ301MPort>(Vec(cols[t], 348), module, FOO::kRightInput1 + t));
        }

        addParam(createParamCentered<RmKnob24>(Vec(mixCol, 168), module, FOO::kLevelParamMix));
        addInput(createInputCentered<PJ301MPort>(Vec(mixCol, 198), module, FOO::kLevelInputMix));
        addParam(createParamCentered<RmToggleButton>(Vec(mixCol, 228), module, FOO::kMuteParamMix));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 318), module, FOO::kLeftOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(mixCol, 348), module, FOO::kRightOutput));
    }
};

Model* modelFOO = createModel<FOO, FOOWidget>("FOO");
