#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define CLIP_DEBUG

//--------------------------------------------------------------
// CLIP
//--------------------------------------------------------------

struct CLIP : Module {

    Amplitude levelAmp;
    MonoTrack monoTrack;

#ifdef CLIP_DEBUG
    float debug1;
    float debug2;
    float debug3;
    float debug4;
#endif

    enum ParamId { kLevelParam, kParamsLen };

    enum InputId { kInput, kLevelCvInput, kInputsLen };

    enum OutputId {
        kOutput,

#ifdef CLIP_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    CLIP() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam<LevelParamQuantity>(kLevelParam, 0.0f, 1.0f, 0.75f, "Level", " dB");
        configInput(kLevelCvInput, "Level CV");

        configInput(kInput, "Audio");
        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef CLIP_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif

        //------------------------------------------------------

        monoTrack.init(
            &(inputs[kInput]),
            &(inputs[kLevelCvInput]));
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        levelAmp.onSampleRateChange(e.sampleRate);
        monoTrack.onSampleRateChange(e.sampleRate);
    }

    //void processOutput(Output& outputs[kOutput], MonoTrack& trk) {
    //    if (outputs[kOutput].isConnected()) {
    //        outputs[kOutput].setChannels(trk.channels);
    //        outputs[kOutput].writeVoltages(trk.voltages);
    //    } else {
    //        outputs[kOutput].setChannels(0);
    //    }
    //}

    void process(const ProcessArgs& args) override {

        if (inputs[kInput].isConnected()) {

            float amp = levelAmp.next(levelToDb(params[kLevelParam].getValue()));
            bool applyLevelCv = inputs[kLevelCvInput].isConnected();

            monoTrack.process(amp, applyLevelCv);

            if (outputs[kOutput].isConnected()) {
                outputs[kOutput].setChannels(monoTrack.channels);
                outputs[kOutput].writeVoltages(monoTrack.voltages);
            } else {
                outputs[kOutput].setChannels(0);
            }
        } else {
            monoTrack.vuStats.process(0.0f);
            outputs[kOutput].setChannels(0);
        }
    }
};

//--------------------------------------------------------------
// CLIPWidget
//--------------------------------------------------------------

struct CLIPWidget : ModuleWidget {

    CLIPWidget(CLIP* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/CLIP.svg")));

        // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

#ifdef CLIP_DEBUG
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 12), module, CLIP::kDebug1));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 36), module, CLIP::kDebug2));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 60), module, CLIP::kDebug3));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 84), module, CLIP::kDebug4));
#endif

        addMeter(24 - 6, 44, module ? &(module->monoTrack.vuStats) : NULL);
        addMeter(24 + 1, 44, module ? &(module->monoTrack.vuStats) : NULL);

        addParam(createParamCentered<MKnob24>(Vec(22.5, 186), module, CLIP::kLevelParam));
        addInput(createInputCentered<MPolyPort>(Vec(22.5, 222), module, CLIP::kLevelCvInput));

        addInput(createInputCentered<MPolyPort>(Vec(22.5, 293), module, CLIP::kInput));
        addOutput(createOutputCentered<MPolyPort>(Vec(22.5, 334), module, CLIP::kOutput));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelCLIP = createModel<CLIP, CLIPWidget>("CLIP");
