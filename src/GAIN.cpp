#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    StereoTrack track;

    enum ParamId { kLevelParam, kMuteParam, kParamsLen };

    enum InputId { kLeftInput, kRightInput, kLevelCvInput, kInputsLen };

    enum OutputId {
        kLeftOutput,
        kRightOutput,

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

        configInput(kLeftInput, "Left");
        configInput(kRightInput, "Right");

        configOutput(kLeftOutput, "Left");
        configOutput(kRightOutput, "Right");

#ifdef GAIN_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif

        //------------------------------------------------------

        track.init(
            &(inputs[kLeftInput]),
            &(inputs[kRightInput]),
            &(params[kLevelParam]),
            &(inputs[kLevelCvInput]),
            NULL,
            NULL);
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        track.onSampleRateChange(e.sampleRate);
    }

    void processOutput(MonoTrack& trk, Output& output) {
        if (output.isConnected()) {
            output.setChannels(trk.output.channels);
            output.writeVoltages(trk.output.voltages);
        } else {
            output.setChannels(0);
        }
    }

    void process(const ProcessArgs& args) override {

        bool muted = params[kMuteParam].getValue() > 0.5f;
        track.process(args.sampleTime, muted);

        processOutput(track.left, outputs[kLeftOutput]);
        processOutput(track.right, outputs[kRightOutput]);
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

        addMeter(24 - 6, 44, module ? &(module->track.left.vuStats) : NULL);
        addMeter(24 + 1, 44, module ? &(module->track.right.vuStats) : NULL);

        addParam(createParamCentered<ArcKnob24>(Vec(24, 166), module, GAIN::kLevelParam));
        addInput(createInputCentered<ArcPolyPort>(Vec(24, 196), module, GAIN::kLevelCvInput));
        addParam(createParamCentered<ArcMuteButton>(Vec(24, 226), module, GAIN::kMuteParam));

        addInput(createInputCentered<ArcPolyPort>(Vec(24, 256), module, GAIN::kLeftInput));
        addInput(createInputCentered<ArcPolyPort>(Vec(24, 286), module, GAIN::kRightInput));

        addOutput(createOutputCentered<ArcPolyPort>(Vec(24, 316), module, GAIN::kLeftOutput));
        addOutput(createOutputCentered<ArcPolyPort>(Vec(24, 346), module, GAIN::kRightOutput));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
