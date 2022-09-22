#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define GAIN_DEBUG

//--------------------------------------------------------------
// GAIN
//--------------------------------------------------------------

struct GAIN : Module {

    StereoTrack stereoTrack;

#ifdef GAIN_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

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
        configSwitch(kMuteParam, 0.f, 1.f, 0.f, "Mute", {"Off", "On"});
        configInput(kLevelCvInput, "Level CV");

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

        stereoTrack.init(
            &(inputs[kLeftInput]),
            &(inputs[kRightInput]),
            &(params[kLevelParam]),
            &(inputs[kLevelCvInput]),
            &(params[kMuteParam]));
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        stereoTrack.onSampleRateChange(e.sampleRate);
    }

    void processOutput(Output& output, MonoTrack& trk) {
        if (output.isConnected()) {
            output.setChannels(trk.channels);
            output.writeVoltages(trk.voltages);
        } else {
            output.setChannels(0);
        }
    }

    void process(const ProcessArgs& args) override {
        stereoTrack.process();

        processOutput(outputs[kLeftOutput], stereoTrack.left);
        processOutput(outputs[kRightOutput], stereoTrack.right);
    }
};

//--------------------------------------------------------------
// GAINWidget
//--------------------------------------------------------------

struct GAINWidget : ModuleWidget {

    GAINWidget(GAIN* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GAIN.svg")));

        // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef GAIN_DEBUG
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 12), module, GAIN::kDebug1));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 36), module, GAIN::kDebug2));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 60), module, GAIN::kDebug3));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 84), module, GAIN::kDebug4));
#endif

        addMeter(24 - 6, 44, module ? &(module->stereoTrack.left.vuStats) : NULL);
        addMeter(24 + 1, 44, module ? &(module->stereoTrack.right.vuStats) : NULL);

        // [168, 198, 228, 258, 288, 318, 348]
        addParam(createParamCentered<MKnob24>(Vec(24, 166), module, GAIN::kLevelParam));
        addInput(createInputCentered<MPolyPort>(Vec(24, 196), module, GAIN::kLevelCvInput));
        addParam(createParamCentered<MToggleButton>(Vec(24, 226), module, GAIN::kMuteParam));

        addInput(createInputCentered<MPolyPort>(Vec(24, 256), module, GAIN::kLeftInput));
        addInput(createInputCentered<MPolyPort>(Vec(24, 286), module, GAIN::kRightInput));

        addOutput(createOutputCentered<MPolyPort>(Vec(24, 316), module, GAIN::kLeftOutput));
        addOutput(createOutputCentered<MPolyPort>(Vec(24, 346), module, GAIN::kRightOutput));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelGAIN = createModel<GAIN, GAINWidget>("GAIN");
