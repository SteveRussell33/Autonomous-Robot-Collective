#include "foo.hpp"
#include "plugin.hpp"
#include "widgets.hpp"

// define FOO_DEBUG

//--------------------------------------------------------------
// FOO
//--------------------------------------------------------------

struct FOO : Module {

    static const int kNumTracks = 2;

    StereoTrack tracks[kNumTracks];

    dsp::ClockDivider ledDivider;

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

        kSendLeftOutput,
        kSendRightOutput,
        kMixLeftOutput,
        kMixRightOutput,

#ifdef FOO_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    enum LightIds {
        ENUMS(kLeftLights1, 8),
        ENUMS(kLeftLights2, 8),
        ENUMS(kLeftLightsMix, 8),

        ENUMS(kRightLights1, 8),
        ENUMS(kRightLights2, 8),
        ENUMS(kRightLightsMix, 8),
        kLightsLen
    };

    FOO() {
        config(kParamsLen, kInputsLen, kOutputsLen, kLightsLen);

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

        configOutput(kSendLeftOutput, "Send Left");
        configOutput(kSendRightOutput, "Send Right");

        configOutput(kMixLeftOutput, "Mix Left");
        configOutput(kMixRightOutput, "Mix Right");

#ifdef FOO_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif

        ledDivider.setDivision(512);
    }

    void process(const ProcessArgs& args) override {

        // Process each track
        for (int t = 0; t < kNumTracks; t++) {
            tracks[t].process(args.sampleTime, inputs[kLeftInput1 + t], inputs[kRightInput1 + t]);
        }

        // Update leds
        if (ledDivider.process()) {
            for (int t = 0; t < kNumTracks; t++) {
                tracks[t].left.updateLeds(lights, kLeftLights1 + t * 8);
                //tracks[t].right.updateLeds(lights, kRightLights1 + t * 8);
            }
        }
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
        addOutput(createOutputCentered<RmPolyPort24>(Vec(12, 12), module, FOO::kDebug1));
        addOutput(createOutputCentered<RmPolyPort24>(Vec(12, 36), module, FOO::kDebug2));
        addOutput(createOutputCentered<RmPolyPort24>(Vec(12, 60), module, FOO::kDebug3));
        addOutput(createOutputCentered<RmPolyPort24>(Vec(12, 84), module, FOO::kDebug4));
#endif

        //////////////////////////////////////////////////////////////

        static const int cols[] = {31, 72};
        static const int mixCol = 113;

        // [168, 198, 228, 258, 288, 318, 348]

        addLights(module, FOO::kLeftLights1, cols[0] - 6, 44);
        addLights(module, FOO::kRightLights1, cols[0] + 6, 44);
        addLights(module, FOO::kLeftLights2, cols[1] - 6, 44);
        addLights(module, FOO::kRightLights2, cols[1] + 6, 44);
        addLights(module, FOO::kLeftLightsMix, mixCol - 6, 44);
        addLights(module, FOO::kRightLightsMix, mixCol + 6, 44);

        for (int t = 0; t < FOO::kNumTracks; t++) {
            addParam(createParamCentered<RmKnob24>(Vec(cols[t], 168), module, FOO::kLevelParam1 + t));
            addInput(createInputCentered<RmPolyPort24>(Vec(cols[t], 198), module, FOO::kLevelInput1 + t));
            addParam(createParamCentered<RmToggleButton>(Vec(cols[t], 228), module, FOO::kMuteParam1 + t));
            addParam(createParamCentered<RmKnob24>(Vec(cols[t], 258), module, FOO::kPanParam1 + t));
            addInput(createInputCentered<RmPolyPort24>(Vec(cols[t], 288), module, FOO::kPanParamInput1 + t));
            addInput(createInputCentered<RmPolyPort24>(Vec(cols[t], 318), module, FOO::kLeftInput1 + t));
            addInput(createInputCentered<RmPolyPort24>(Vec(cols[t], 348), module, FOO::kRightInput1 + t));
        }

        addParam(createParamCentered<RmKnob24>(Vec(mixCol, 168), module, FOO::kLevelParamMix));
        addInput(createInputCentered<RmPolyPort24>(Vec(mixCol, 198), module, FOO::kLevelInputMix));
        addParam(createParamCentered<RmToggleButton>(Vec(mixCol, 228), module, FOO::kMuteParamMix));
        addOutput(createOutputCentered<RmMonoPort24>(Vec(mixCol, 258), module, FOO::kSendLeftOutput));
        addOutput(createOutputCentered<RmMonoPort24>(Vec(mixCol, 288), module, FOO::kSendRightOutput));
        addOutput(createOutputCentered<RmMonoPort24>(Vec(mixCol, 318), module, FOO::kMixLeftOutput));
        addOutput(createOutputCentered<RmMonoPort24>(Vec(mixCol, 348), module, FOO::kMixRightOutput));
    }

    void addLights(FOO* module, int lightID, int x, int y) {
        addChild(createLightCentered<SmallSimpleLight<RedLight>>(Vec(x, y), module, lightID));
        addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(x, y + 14), module, lightID + 1));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 28), module, lightID + 2));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 42), module, lightID + 3));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 56), module, lightID + 4));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 70), module, lightID + 5));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 84), module, lightID + 6));
        addChild(createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x, y + 98), module, lightID + 7));
    }
};

Model* modelFOO = createModel<FOO, FOOWidget>("FOO");
