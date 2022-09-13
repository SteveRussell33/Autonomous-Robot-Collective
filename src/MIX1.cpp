#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define MIX1_DEBUG

//--------------------------------------------------------------
// MIX1
//--------------------------------------------------------------

struct MIX1 : Module {

    // Amplitude faderAmp;
    // Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

    StereoLevels levels;

#ifdef MIX1_DEBUG
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

        kLeftInput,
        kRightInput,
        kLevelInput,

        kInputsLen
    };

    enum OutputId {

        kLeftOutput,
        kRightOutput,

#ifdef MIX1_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    MIX1() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam<FaderParamQuantity>(kFader, 0.0f, 1.0f, kFaderDbZero, "Fader", " dB");

        configSwitch(kMute, 0.f, 1.f, 0.f, "Mute", {"Off", "On"});

        configInput(kLevelInput, "Level CV");
        configInput(kLeftInput, "Left");
        configInput(kRightInput, "Right");

        configOutput(kLeftOutput, "Left");
        configOutput(kRightOutput, "Right");

#ifdef MIX1_DEBUG
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
// MIX1Widget
//--------------------------------------------------------------

struct MIX1Widget : ModuleWidget {

    MIX1Widget(MIX1* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MIX1.svg")));

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef MIX1_DEBUG
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 12), module, MIX1::kDebug1));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 36), module, MIX1::kDebug2));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 60), module, MIX1::kDebug3));
        addOutput(createOutputCentered<PJ301MPort>(Vec(12, 84), module, MIX1::kDebug4));
#endif

        //////////////////////////////////////////////////////////////

        static const int faderY = 39;

        addFader(30, faderY, MIX1::kFader);
        addMeter(30, faderY, module ?  &(module->levels) : NULL);
        addParam(createParamCentered<RmToggleButton>(Vec(30, 203), module, MIX1::kMute));
        addInput(createInputCentered<PJ301MPort>(Vec(30, 232), module, MIX1::kLevelInput));
        addInput(createInputCentered<PJ301MPort>(Vec(30, 261), module, MIX1::kLeftInput));
        addInput(createInputCentered<PJ301MPort>(Vec(30, 290), module, MIX1::kRightInput));

        addOutput(createOutputCentered<PJ301MPort>(Vec(30, 319), module, MIX1::kLeftOutput));
        addOutput(createOutputCentered<PJ301MPort>(Vec(30, 348), module, MIX1::kRightOutput));
    }

    void addFader(float x, float y, int faderID) {
        static const float faderXofs = 4.5;
        static const float faderYofs = -9.5;

        addParam(createParam<RmFader>(Vec(x + faderXofs, y + faderYofs), module, faderID));
    }

    void addMeter(float x, float y, StereoLevels* levels) {
        static const float meterXofs = -2.5;
        static const float meterH = 9;
        static const float meterW = 144;

        VUMeter* meter = new VUMeter(levels);
        meter->box.pos = Vec(x + meterXofs, y);
        meter->box.size = Vec(meterH, meterW);
        addChild(meter);
    }

};

Model* modelMIX1 = createModel<MIX1, MIX1Widget>("MIX1");
