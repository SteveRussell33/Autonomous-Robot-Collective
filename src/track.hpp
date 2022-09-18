#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

#include "vu.hpp"

using namespace rack;

//--------------------------------------------------------------
// FaderParamQuantity
//--------------------------------------------------------------

static float faderToDb(float v) {
    // clang-format off
    if      (v >= 0.5) return rescale(v, 0.5f, 1.0f, -12.0f,  12.0f);
    else if (v >= 0.2) return rescale(v, 0.2f, 0.5f, -36.0f, -12.0f);
    else               return rescale(v, 0.0f, 0.2f, -60.0f, -36.0f);
    // clang-format on
}

struct FaderParamQuantity : ParamQuantity {

    float getDisplayValue() override {
        float v = getValue();
        if (!module) {
            return v;
        }

        return faderToDb(v);
    }

    void setDisplayValue(float v) override {
        if (!module) {
            return;
        }

        v = clamp(v, -60.0f, 12.0f);

        // clang-format off
        if      (v >= -12.0f) v =  rescale(v, -12.0f,  12.0f, 0.5f, 1.0f);
        else if (v >= -36.0f) v =  rescale(v, -36.0f, -12.0f, 0.2f, 0.5f);
        else                  v =  rescale(v, -60.0f, -36.0f, 0.0f, 0.2f);
        // clang-format on

        setValue(v);
    }
};

//--------------------------------------------------------------
// Amplitude
//--------------------------------------------------------------

static const float kMuteDb = -120.0f;
static const float kMinDb = -60.0f;
static const float kMaxDb = 12.0f;

struct Amplitude {

  private:

    float curDb;
    float curAmp;
    bogaudio::dsp::SlewLimiter slew;

  public:

    Amplitude() {
        curDb = kMinDb;
        // TODO use a lookup table
        curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        slew.setLast(curDb);
    }

    void onSampleRateChange(float sampleRate) {
        slew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
    }

    float next(float db) {

        float dbs = slew.next(db);
        if (curDb != dbs) {
            curDb = dbs;

            // TODO use a lookup table
            curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        }
        return curAmp;
    }
};

//--------------------------------------------------------------
// Pan
//--------------------------------------------------------------

struct Pan {

  private:

    float curPan = 0.0f;

    bogaudio::dsp::SlewLimiter slew;

  public:

    float left = 0.7071068f;
    float right = 0.7071068f;

    Pan() {
        slew.setLast(0.0f);
    }

    void onSampleRateChange(float sampleRate) {
        slew.setParams(sampleRate, 5.0f, 2.0f);
    }

    void next(float pan) {

        pan = clamp(pan, -1.0f, 1.0f);

        float ps = slew.next(pan);
        if (curPan != ps) {
            curPan = ps;

            float p = (curPan + 1.0f) * 0.125f;

            // TODO use lookup tables
            left = std::cosf(2.0f * M_PI * p);
            right = std::sinf(2.0f * M_PI * p);
        }
    }
};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

  private:

    Amplitude levelAmps[engine::PORT_MAX_CHANNELS];

    float nextFaderCvAmp(Input& faderCvInput, int ch) {
        float v = faderCvInput.getPolyVoltage(ch);
        float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        return levelAmps[ch].next(db);
    }

  public:

    float sum = 0.0f;
    VuLevel vuLevel;

    void onSampleRateChange(float sampleRate) {
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelAmps[ch].onSampleRateChange(sampleRate);
        }
        vuLevel.onSampleRateChange(sampleRate);
    }

    void amplify(Input& input, Input& faderCvInput, float ampF, bool applyFaderCv, Output& output) {

        int channels = std::max(input.getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {

            float ampCh = ampF;
            if (applyFaderCv) {
                ampCh = ampCh * nextFaderCvAmp(faderCvInput, ch);
            }

            // hard clip
            float out = clamp(input.getPolyVoltage(ch) * ampCh, -10.0f, 10.0f);

            output.setVoltage(out, ch);
        }
        output.setChannels(channels);
    }

    // void pan() {
    // }

    void summarize(Output& output) {
        sum = output.getVoltageSum();
        vuLevel.process(sum);
    }

    void disconnect(Output& output) {
        output.setChannels(0);
        sum = 0.0f;
        vuLevel.process(0.0f);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

struct StereoTrack {

  private:

    Amplitude faderAmp;

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        faderAmp.onSampleRateChange(sampleRate);
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(
        Input& leftInput,
        Input& rightInput,
        Param& faderParam,
        Input& faderCvInput,
        Param& muteParam,
        Output& leftOutput,
        Output& rightOutput) {

        bool muted = muteParam.getValue() > 0.5f;

        // fader amplitude
        float ampF = 0.0f;
        if (muted) {
            ampF = faderAmp.next(kMuteDb);
        } else {
            ampF = faderAmp.next(faderToDb(faderParam.getValue()));
        }

        // fader cv
        bool applyFaderCv = (!muted && faderCvInput.isConnected());

        // left connected
        if (leftInput.isConnected()) {
            left.amplify(leftInput, faderCvInput, ampF, applyFaderCv, leftOutput);
            // left.pan();
            left.summarize(leftOutput);
        }
        // left disconnected
        else {
            left.disconnect(leftOutput);
        }

        // right connected
        if (rightInput.isConnected()) {
            right.amplify(rightInput, faderCvInput, ampF, applyFaderCv, rightOutput);
            // right.pan();
            right.summarize(rightOutput);
        }
        // right disconnected
        else {
            right.disconnect(rightOutput);
        }
    }
};
