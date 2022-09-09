#include "pub/pub.hpp"

#include "oversample.hpp"
#include "plugin.hpp"
#include "widgets.hpp"

// define SAT_DEBUG

struct SAT : Module {

    const int kOversampleFactor = 4;
    Oversample oversample{kOversampleFactor};

    enum ParamId {
        kDriveParam,
        kDriveCvAmountParam,

        kParamsLen
    };

    enum InputId {
        kDriveCvInput,
        kInput,

        kInputsLen
    };

    enum OutputId {
        kOutput,

#ifdef SAT_DEBUG
        kDebug1,
        kDebug2,
        kDebug3,
        kDebug4,
#endif
        kOutputsLen
    };

    SAT() {
        config(kParamsLen, kInputsLen, kOutputsLen, 0);

        configParam(kDriveParam, 0.0f, 10.0f, 0.0f, "Drive");
        configParam(kDriveCvAmountParam, -1.0f, 1.0f, 0.0f, "Drive CV amount");

        configInput(kDriveCvInput, "Drive CV");
        configInput(kInput, "Audio");

        configOutput(kOutput, "Audio");

        configBypass(kInput, kOutput);

#ifdef SAT_DEBUG
        configOutput(kDebug1, "Debug 1");
        configOutput(kDebug2, "Debug 2");
        configOutput(kDebug3, "Debug 3");
        configOutput(kDebug4, "Debug 4");
#endif
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        oversample.sampleRateChange(e.sampleRate);
    }

    inline float saturate(float in, float drive) {
        return in * (1 - drive) + fastTanh(in * M_PI) * drive;
    }

    float oversampleDrive(float in, float drive) {

        float buffer[kMaxOversample] = {};
        oversample.upsample(in, buffer);

        for (int i = 0; i < kOversampleFactor; i++) {
            buffer[i] = saturate(buffer[i], drive);
        }

        return oversample.downsample(buffer);
    }

    void process(const ProcessArgs& args) override {
        if (!outputs[kOutput].isConnected()) {
            return;
        }

        float pDrive = params[kDriveParam].getValue() / 10.0f;
        float pDriveCvAmount = params[kDriveCvAmountParam].getValue();

        int channels = std::max(inputs[kInput].getChannels(), 1);

        for (int ch = 0; ch < channels; ch++) {

            float inDriveCv = inputs[kDriveCvInput].getPolyVoltage(ch);
            float drive = pDrive + inDriveCv * pDriveCvAmount;
            drive = clamp(drive, 0.0, 10.0);

            float in = inputs[kInput].getPolyVoltage(ch) / 5.0f;

            float out = oversampleDrive(in, drive);

            outputs[kOutput].setVoltage(out * 5.0f, ch);
        }
        outputs[kOutput].setChannels(channels);
    }
};

struct SATWidget : ModuleWidget {
    SATWidget(SAT* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/SAT.svg")));

#ifdef SAT_DEBUG
        addOutput(createOutputCentered<MPort>(Vec(12, 12), module, SAT::kDebug1));
        addOutput(createOutputCentered<MPort>(Vec(12, 36), module, SAT::kDebug2));
        addOutput(createOutputCentered<MPort>(Vec(12, 60), module, SAT::kDebug3));
        addOutput(createOutputCentered<MPort>(Vec(12, 84), module, SAT::kDebug4));
#endif

        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        addParam(createParamCentered<MKnob32>(Vec(22.5, 78), module, SAT::kDriveParam));
        addParam(createParamCentered<MKnob18>(Vec(22.5, 120), module, SAT::kDriveCvAmountParam));
        addInput(createInputCentered<MPort>(Vec(22.5, 162), module, SAT::kDriveCvInput));

        addInput(createInputCentered<MPort>(Vec(22.5, 292), module, SAT::kInput));
        addOutput(createOutputCentered<MPort>(Vec(22.5, 334), module, SAT::kOutput));
    }
};

Model* modelSAT = createModel<SAT, SATWidget>("SAT");
