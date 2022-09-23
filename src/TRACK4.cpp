#include "plugin.hpp"
#include "track.hpp"
#include "widgets.hpp"

// define TRACK4_DEBUG

//--------------------------------------------------------------
// TRACK4
//--------------------------------------------------------------

struct TRACK4 : Module {

    static const int kNumTracks = 4;

    StereoTrack tracks[kNumTracks];

    StereoTrack mix;
    Input mixLeftInput;
    Input mixRightInput;

    enum ParamId {
        ENUMS(kLevelParam, kNumTracks),
        ENUMS(kMuteParam, kNumTracks),
        ENUMS(kPanParam, kNumTracks),
        kMixLevelParam,
        kMixMuteParam,
        kMixPanParam,
        kParamsLen
    };

    enum InputId {
        ENUMS(kLeftInput, kNumTracks),
        ENUMS(kRightInput, kNumTracks),
        ENUMS(kLevelCvInput, kNumTracks),
        ENUMS(kPanCvInput, kNumTracks),
        kMixLevelCvInput,
        kMixPanCvInput,
        kInputsLen
    };

    enum OutputId {
        ENUMS(kLeftSend, kNumTracks),
        ENUMS(kRightSend, kNumTracks),
        kMixLeftSend,
        kMixRightSend,
        kMixLeftOutput,
        kMixRightOutput,

#ifdef TRACK4_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    TRACK4() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        for (int t = 0; t < kNumTracks; t++) {
            configParam<LevelParamQuantity>(
                kLevelParam + t, 0.0f, 1.0f, 0.75f, string::f("Track %d Level", t + 1), " dB");
            configInput(kLevelCvInput + t, string::f("Track %d Level CV", t + 1));
            configSwitch(
                kMuteParam + t,
                0.f,
                1.f,
                0.f,
                string::f("Track %d Mute/Solo", t + 1),
                {"Off", "On"});

            configParam(kPanParam + t, 0.0f, 1.0f, 0.5f, string::f("Track %d Pan", t + 1));
            configInput(kPanCvInput + t, string::f("Track %d Pan CV", t + 1));

            configInput(kLeftInput + t, string::f("Track %d Left", t + 1));
            configInput(kRightInput + t, string::f("Track %d Right", t + 1));

            configOutput(kLeftSend + t, string::f("Send %d Left", t + 1));
            configOutput(kRightSend + t, string::f("Send %d Right", t + 1));
        }

        configParam<LevelParamQuantity>(kMixLevelParam, 0.0f, 1.0f, 0.75f, "Mix Level", " dB");
        configInput(kMixLevelCvInput, "Mix Level CV");
        configSwitch(kMixMuteParam, 0.f, 1.f, 0.f, "Mix Mute", {"Off", "On"});

        configParam(kMixPanParam, 0.0f, 1.0f, 0.5f, "Mix Pan");
        configInput(kMixPanCvInput, "Mix Pan CV");

        configOutput(kMixLeftSend, "Send Mix Left");
        configOutput(kMixRightSend, "Send Mix Right");

        configOutput(kMixLeftOutput, "Mix Left");
        configOutput(kMixRightOutput, "Mix Right");

#ifdef TRACK4_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif

        //------------------------------------------------------

        for (int t = 0; t < TRACK4::kNumTracks; t++) {
            tracks[t].init(
                &(inputs[kLeftInput + t]),
                &(inputs[kRightInput + t]),
                &(params[kLevelParam + t]),
                &(inputs[kLevelCvInput + t]));
        }

        mix.init(
            &mixLeftInput, &mixRightInput, &(params[kMixLevelParam]), &(inputs[kMixLevelCvInput]));

        mixLeftInput.channels = kNumTracks;
        mixRightInput.channels = kNumTracks;
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        for (int t = 0; t < TRACK4::kNumTracks; t++) {
            tracks[t].onSampleRateChange(e.sampleRate);
        }
        mix.onSampleRateChange(e.sampleRate);
    }

    void processSend(Port& port, Output& send) {
        if (send.isConnected()) {
            send.setChannels(port.channels);
            send.writeVoltages(port.voltages);
        } else {
            send.setChannels(0);
        }
    }

    void processMixOutput(float val, Output& output) {
        if (output.isConnected()) {
            output.setChannels(1);
            output.setVoltage(val, 0);
        } else {
            output.setChannels(0);
        }
    }

    void process(const ProcessArgs& args) override {

        for (int t = 0; t < TRACK4::kNumTracks; t++) {
            bool muted = params[kMuteParam + t].getValue() > 0.5f;
            tracks[t].process(muted);

            processSend(tracks[t].left.output, outputs[kLeftSend + t]);
            processSend(tracks[t].right.output, outputs[kRightSend + t]);

            mixLeftInput.setVoltage(tracks[t].left.sum, t);
            mixRightInput.setVoltage(tracks[t].right.sum, t);
        }

        processSend(mixLeftInput, outputs[kMixLeftSend]);
        processSend(mixRightInput, outputs[kMixRightSend]);

        bool muted = params[kMixMuteParam].getValue() > 0.5f;
        mix.process(muted);
        processMixOutput(mix.left.sum, outputs[kMixLeftOutput]);
        processMixOutput(mix.right.sum, outputs[kMixRightOutput]);
    }
};

//--------------------------------------------------------------
// TRACK4Widget
//--------------------------------------------------------------

struct TRACK4Widget : ModuleWidget {

    TRACK4Widget(TRACK4* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/TRACK4.svg")));

        addChild(createWidget<ScrewSilver>(Vec(15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(15, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

#ifdef TRACK4_DEBUG
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 12), module, TRACK4::kDebug1));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 36), module, TRACK4::kDebug2));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 60), module, TRACK4::kDebug3));
        addOutput(createOutputCentered<MPolyPort>(Vec(12, 84), module, TRACK4::kDebug4));
#endif

        int x = 30;
        for (int t = 0; t < TRACK4::kNumTracks; t++) {
            addMeter(x - 6, 44, module ? &(module->tracks[t].left.vuStats) : NULL);
            addMeter(x + 1, 44, module ? &(module->tracks[t].right.vuStats) : NULL);

            addParam(createParamCentered<MKnob24>(Vec(x, 166), module, TRACK4::kLevelParam + t));
            addInput(
                createInputCentered<MPolyPort>(Vec(x, 196), module, TRACK4::kLevelCvInput + t));
            addParam(
                createParamCentered<MToggleButton>(Vec(x, 226), module, TRACK4::kMuteParam + t));

            addParam(createParamCentered<MKnob24>(Vec(x, 256), module, TRACK4::kPanParam + t));
            addInput(createInputCentered<MPolyPort>(Vec(x, 286), module, TRACK4::kPanCvInput + t));

            addInput(createInputCentered<MPolyPort>(Vec(x, 316), module, TRACK4::kLeftInput + t));
            addInput(createInputCentered<MPolyPort>(Vec(x, 346), module, TRACK4::kRightInput + t));
            x += 38;
        }

        addMeter(x - 6, 44, module ? &(module->mix.left.vuStats) : NULL);
        addMeter(x + 1, 44, module ? &(module->mix.right.vuStats) : NULL);

        addParam(createParamCentered<MKnob24>(Vec(x, 166), module, TRACK4::kMixLevelParam));
        addInput(createInputCentered<MPolyPort>(Vec(x, 196), module, TRACK4::kMixLevelCvInput));
        addParam(createParamCentered<MToggleButton>(Vec(x, 226), module, TRACK4::kMixMuteParam));

        addParam(createParamCentered<MKnob24>(Vec(x, 256), module, TRACK4::kMixPanParam));
        addInput(createInputCentered<MPolyPort>(Vec(x, 286), module, TRACK4::kMixPanCvInput));

        addOutput(createOutputCentered<MPolyPort>(Vec(x, 316), module, TRACK4::kMixLeftOutput));
        addOutput(createOutputCentered<MPolyPort>(Vec(x, 346), module, TRACK4::kMixRightOutput));

        // sends
        int y = 51;
        for (int t = 0; t < TRACK4::kNumTracks; t++) {
            addOutput(createOutputCentered<MPolyPort>(Vec(234, y), module, TRACK4::kLeftSend + t));
            addOutput(
                createOutputCentered<MPolyPort>(Vec(234, y + 29), module, TRACK4::kRightSend + t));
            y += 67;
        }
        addOutput(createOutputCentered<MPolyPort>(Vec(234, y), module, TRACK4::kMixLeftSend));
        addOutput(createOutputCentered<MPolyPort>(Vec(234, y + 29), module, TRACK4::kMixRightSend));
    }

    void addMeter(float x, float y, VuStats* vuStats) {
        VuMeter* meter = new VuMeter(vuStats);
        meter->box.pos = Vec(x, y);
        meter->box.size = Vec(8, 104);
        addChild(meter);
    }
};

Model* modelTRACK4 = createModel<TRACK4, TRACK4Widget>("TRACK4");
