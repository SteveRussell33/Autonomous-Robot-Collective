#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

#include "vu.hpp"

using namespace rack;

//--------------------------------------------------------------
// LevelParamQuantity
//--------------------------------------------------------------

static float levelToDb(float v) {
    // clang-format off
    if      (v >= 0.5) return rescale(v, 0.5f, 1.0f, -12.0f,  12.0f);
    else if (v >= 0.2) return rescale(v, 0.2f, 0.5f, -36.0f, -12.0f);
    else               return rescale(v, 0.0f, 0.2f, -60.0f, -36.0f);
    // clang-format on
}

struct LevelParamQuantity : ParamQuantity {

    float getDisplayValue() override {
        float v = getValue();
        if (!module) {
            return v;
        }

        return levelToDb(v);
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
        curAmp = 0.0f;
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

    Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];

    Input* input;
    Input* levelCvInput;

    float nextLevelCvAmp(Input* levelCvInput, int ch) {
        float v = levelCvInput->getPolyVoltage(ch);
        float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        return levelCvAmps[ch].next(db);
    }

    void amplify(float amp, bool applyLevelCv) {

        channels = std::max(input->getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {

            float chAmp = amp;
            if (applyLevelCv) {
                chAmp = chAmp * nextLevelCvAmp(levelCvInput, ch);
            }

            // hard clip
            voltages[ch] = clamp(input->getPolyVoltage(ch) * chAmp, -10.0f, 10.0f);
        }
    }

    // void pan() {
    // }

    void updateStats() {
        float sum = 0.f;
        for (int c = 0; c < channels; c++) {
            sum += voltages[c];
        }
        vuStats.process(sum);
    }

  public:

    int channels = 0;
    float voltages[engine::PORT_MAX_CHANNELS] = {};
    VuStats vuStats;

    void init(Input* input_, Input* levelCvInput_) {

        input = input_;
        levelCvInput = levelCvInput_;
    }

    void onSampleRateChange(float sampleRate) {
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelCvAmps[ch].onSampleRateChange(sampleRate);
        }
        vuStats.onSampleRateChange(sampleRate);
    }

    void process(float amp, bool applyLevelCv) {

        amplify(amp, applyLevelCv);
        // pan();
        updateStats();
    }

    void copyFrom(MonoTrack& trk) {
        float sum = 0.f;
        channels = trk.channels;
        for (int c = 0; c < channels; c++) {
            voltages[c] = trk.voltages[c];
            sum += voltages[c];
        }
        vuStats.process(sum);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

struct StereoTrack {

  private:

    Amplitude levelAmp;

    Input* leftInput = NULL;
    Input* rightInput = NULL;
    Param* levelParam = NULL;
    Input* levelCvInput = NULL;
    Param* muteParam = NULL;

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        levelAmp.onSampleRateChange(sampleRate);
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void init(
        Input* leftInput_,
        Input* rightInput_,
        Param* levelParam_,
        Input* levelCvInput_,
        Param* muteParam_) {

        leftInput = leftInput_;
        rightInput = rightInput_;
        levelParam = levelParam_;
        levelCvInput = levelCvInput_;
        muteParam = muteParam_;

        left.init(leftInput, levelCvInput);
        right.init(rightInput, levelCvInput);
    }

    void process() {

        bool muted = muteParam->getValue() > 0.5f;

        // level amplitude
        float amp = 0.0f;
        if (muted) {
            amp = levelAmp.next(kMinDb);
        } else {
            amp = levelAmp.next(levelToDb(levelParam->getValue()));
        }

        // level cv
        bool applyLevelCv = (!muted && levelCvInput->isConnected());

        if (leftInput->isConnected()) {
            left.process(amp, applyLevelCv);

            // stereo
            if (rightInput->isConnected()) {
                right.process(amp, applyLevelCv);
            }
            // mono: copy left to right
            else {
                right.copyFrom(left);
            }
        } else {
            // mono: copy right to left
            if (rightInput->isConnected()) {
                right.process(amp, applyLevelCv);
                left.copyFrom(right);
            }
            // no inputs
            else {
                left.vuStats.process(0.0f);
                right.vuStats.process(0.0f);
            }
        }
    }
};
