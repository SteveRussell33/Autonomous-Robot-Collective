#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/signal.hpp"

#include "vu.hpp"

using namespace rack;

static const float kMinDb = -60.0f;
static const float kMaxDb = 12.0f;

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

    void processChannel(Input* input, int ch, float amp) {

        // hard clip
        float out = clamp(input->getPolyVoltage(ch) * amp, -10.0f, 10.0f);

        output.voltages[ch] = out;
        sum += out;
    }

    void disconnect(float sampleTime) {
        sum = 0.f;
        vuStats.process(sampleTime, 0.0f);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

class StereoTrack {

  private:

    Amplitude levelAmp;
    Amplitude levelCvAmps[engine::PORT_MAX_CHANNELS];
    arc::dsp::Panner panner;

    Input* leftInput = NULL;
    Input* rightInput = NULL;

    Param* levelParam = NULL;
    Input* levelCvInput = NULL;

    Param* panParam = NULL;
    Input* panCvInput = NULL;

    float nextLevelCvAmp(int ch) {
        float v = levelCvInput->getPolyVoltage(ch);
        float db = rescale(v, 0.0f, 10.0f, kMinDb, kMaxDb);
        return arc::dsp::decibelsToAmplitude(db);
    }

    void processStereo(float sampleTime, Input* inLeft, Input* inRight, bool muted) {

        left.sum = 0.0f;
        right.sum = 0.0f;

        left.output.channels = std::max(inLeft->getChannels(), 1);
        right.output.channels = std::max(inRight->getChannels(), 1);

        int maxChans = std::max(left.output.channels, right.output.channels);

        if (muted) {
            for (int ch = 0; ch < maxChans; ch++) {
                float m = levelCvAmps[ch].nextMute();
                left.processChannel(inLeft, ch, m);
                right.processChannel(inRight, ch, m);
            }
        } else {
            float amp = levelAmp.next(levelToDb(levelParam->getValue()));

            for (int ch = 0; ch < maxChans; ch++) {
                float leftAmp = amp;
                float rightAmp = amp;

                // level cv
                if (levelCvInput->isConnected()) {
                    float nla = nextLevelCvAmp(ch);
                    leftAmp *= nla;
                    rightAmp *= nla;
                }

                // panning
                float pan = panParam->getValue();
                if (panCvInput->isConnected()) {
                    pan += panCvInput->getPolyVoltage(ch) * 0.2f;
                }
                pan = clamp(pan, -1.0f, 1.0f);

                panner.next(pan);
                leftAmp *= panner.left;
                rightAmp *= panner.right;

                // process left/right
                left.processChannel(inLeft, ch, leftAmp);
                right.processChannel(inRight, ch, rightAmp);
            }
        }

        left.vuStats.process(sampleTime, left.sum * 0.2f);
        right.vuStats.process(sampleTime, right.sum * 0.2f);
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

    void init(
        Input* leftInput_,
        Input* rightInput_,
        Param* levelParam_,
        Input* levelCvInput_,
        Param* panParam_,
        Input* panCvInput_) {

        leftInput = leftInput_;
        rightInput = rightInput_;
        levelParam = levelParam_;
        levelCvInput = levelCvInput_;
        panParam = panParam_;
        panCvInput = panCvInput_;
    }

    void process(float sampleTime, bool muted) {

        if (leftInput->isConnected()) {
            // stereo
            if (rightInput->isConnected()) {
                processStereo(sampleTime, leftInput, rightInput, muted);
            }
            // mono: copy left to right
            else {
                processStereo(sampleTime, leftInput, leftInput, muted);
            }
        } else {
            // mono: copy right to left
            if (rightInput->isConnected()) {
                processStereo(sampleTime, rightInput, rightInput, muted);
            }
            // no inputs
            else {
                left.disconnect(sampleTime);
                right.disconnect(sampleTime);
            }
        }
    }
};
