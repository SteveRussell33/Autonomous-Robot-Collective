#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

#include "vu.hpp"

using namespace rack;

static const float kMinDb = -60.0f;
static const float kMaxDb = 12.0f;
static const float kDecibelRange = kMaxDb - kMinDb;

//--------------------------------------------------------------
// LevelParamQuantity
//--------------------------------------------------------------

static float levelToDb(float v) {
    // TODO compute it more efficiently
    // clang-format off
    if      (v >= 0.5) return rescale(v, 0.5f, 1.0f, -12.0f,  12.0f);
    else if (v >= 0.2) return rescale(v, 0.2f, 0.5f, -36.0f, -12.0f);
    else               return rescale(v, 0.0f, 0.2f, -60.0f, -36.0f);
    // clang-format on
}

struct LevelParamQuantity : ParamQuantity {

private:

    static constexpr float kInv24Db = 1.0f / 24.0f;

    inline float rescaleToDb(float x, float xMin, float yMin, float yMax) {
        return yMin + (x - xMin) * kInv24Db * (yMax - yMin);
    }

public:

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
        if      (v >= -12.0f) v =  rescaleToDb(v, -12.0f, 0.5f, 1.0f);
        else if (v >= -36.0f) v =  rescaleToDb(v, -36.0f, 0.2f, 0.5f);
        else                  v =  rescaleToDb(v, -60.0f, 0.0f, 0.2f);
        // clang-format on

        setValue(v);
    }
};

//--------------------------------------------------------------
// Amplitude
//--------------------------------------------------------------

class Amplitude {

  private:

    float amp;
    bool muted;

    bogaudio::dsp::SlewLimiter dbSlew;
    bogaudio::dsp::SlewLimiter muteSlew;

    bogaudio::dsp::Amplifier amplifier;

  public:

    Amplitude() : amp(0.0f), muted(true) {
        muteSlew.setLast(0.0f);
    }

    void onSampleRateChange(float sampleRate) {
        dbSlew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
        muteSlew.setParams(sampleRate, 25.0f, 1.0f);
    }

    float next(float db) {

        if (muted) {
            // NOTE We don't need a lookup table here.
            dbSlew.setLast(bogaudio::dsp::amplitudeToDecibels(amp));
            muted = false;
        }

        amplifier.setLevel(dbSlew.next(db));
        amp = amplifier._level;
        return amp;
    }

    float nextMute() {

        if (!muted) {
            muteSlew.setLast(amp);
            muted = true;
        }

        amp = muteSlew.next(0.0f);
        return amp;
    }
};

////--------------------------------------------------------------
//// Pan
////--------------------------------------------------------------
//
//class Pan {
//
//  private:
//
//    float curPan = 0.0f;
//
//    bogaudio::dsp::SlewLimiter slew;
//
//  public:
//
//    float left = 0.7071068f;
//    float right = 0.7071068f;
//
//    Pan() {
//        slew.setLast(0.0f);
//    }
//
//    void onSampleRateChange(float sampleRate) {
//        slew.setParams(sampleRate, 5.0f, 2.0f);
//    }
//
//    void next(float pan) {
//
//        pan = clamp(pan, -1.0f, 1.0f);
//
//        float ps = slew.next(pan);
//        if (curPan != ps) {
//            curPan = ps;
//
//            float p = (curPan + 1.0f) * 0.125f;
//
//            // TODO use lookup tables
//            left = std::cosf(2.0f * M_PI * p);
//            right = std::sinf(2.0f * M_PI * p);
//        }
//    }
//};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

    Output output;
    float sum = 0.f;
    VuStats vuStats;

    void onSampleRateChange(float sampleRate) {
        vuStats.onSampleRateChange(sampleRate);
    }

    void process(Input* input, float* amps) {

        sum = 0.0f;
        output.channels = std::max(input->getChannels(), 1);
        for (int ch = 0; ch < output.channels; ch++) {

            // hard clip
            float out = clamp(input->getPolyVoltage(ch) * amps[ch], -10.0f, 10.0f);

            output.voltages[ch] = out;
            sum += out;
        }

        vuStats.process(sum);
    }

    void copyFrom(MonoTrack& trk) {
        output.channels = trk.output.channels;
        output.writeVoltages(trk.output.voltages);
        sum = trk.sum;
        vuStats.copyFrom(trk.vuStats);
    }

    void disconnect() {
        sum = 0.f;
        vuStats.process(0.0f);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

class StereoTrack {

  private:

    Amplitude levelAmp;
    Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];

    float leftAmps[engine::PORT_MAX_CHANNELS];
    float rightAmps[engine::PORT_MAX_CHANNELS];

    Input* leftInput = NULL;
    Input* rightInput = NULL;
    Param* levelParam = NULL;
    Input* levelCvInput = NULL;

    float nextLevelCvAmp(int ch) {
        float v = levelCvInput->getPolyVoltage(ch);
        // rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        float db = kMinDb + v * 0.1f * kDecibelRange;
        return levelCvAmps[ch].next(db);
    }

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        levelAmp.onSampleRateChange(sampleRate);
        for (int ch = 0; ch < engine::PORT_MAX_CHANNELS; ch++) {
            levelCvAmps[ch].onSampleRateChange(sampleRate);
        }
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void init(Input* leftInput_, Input* rightInput_, Param* levelParam_, Input* levelCvInput_) {
        leftInput = leftInput_;
        rightInput = rightInput_;
        levelParam = levelParam_;
        levelCvInput = levelCvInput_;
    }

    void process(bool muted) {

        //--------------------------------------------------
        // Compute the left and right amplitude per-channel

        int maxChans = std::max(leftInput->getChannels(), rightInput->getChannels());
        maxChans = std::max(maxChans, 1);

        if (muted) {
            for (int ch = 0; ch < maxChans; ch++) {
                float ampCh = levelCvAmps[ch].nextMute();
                leftAmps[ch] = ampCh;
                rightAmps[ch] = ampCh;
            }
        } else {
            float amp = levelAmp.next(levelToDb(levelParam->getValue()));

            if (levelCvInput->isConnected()) {
                for (int ch = 0; ch < maxChans; ch++) {
                    float ampCh = amp * nextLevelCvAmp(ch);
                    leftAmps[ch] = ampCh;
                    rightAmps[ch] = ampCh;
                }
            } else {
                for (int ch = 0; ch < maxChans; ch++) {
                    leftAmps[ch] = amp;
                    rightAmps[ch] = amp;
                }
            }
        }

        //--------------------------------------------------
        // Process left and right

        if (leftInput->isConnected()) {
            left.process(leftInput, leftAmps);

            // stereo
            if (rightInput->isConnected()) {
                right.process(rightInput, rightAmps);
            }
            // mono: copy left to right
            else {
                right.copyFrom(left);
            }
        } else {
            // mono: copy right to left
            if (rightInput->isConnected()) {
                right.process(rightInput, rightAmps);
                left.copyFrom(right);
            }
            // no inputs
            else {
                left.disconnect();
                right.disconnect();
            }
        }
    }
};
