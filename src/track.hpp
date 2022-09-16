#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

using namespace rack;

const float kMuteDb = -120.0f;

const float kMinDb = -60.0f;
const float kMaxDb = 12.0f;

//--------------------------------------------------------------
// Volume
//--------------------------------------------------------------

static float volumeToDb(float v) {
    // clang-format off
    if      (v >= 0.5) return rescale(v, 0.5f, 1.0f, -12.0f,  12.0f);
    else if (v >= 0.2) return rescale(v, 0.2f, 0.5f, -36.0f, -12.0f);
    else               return rescale(v, 0.0f, 0.2f, -60.0f, -36.0f);
    // clang-format on
}

struct VolumeParamQuantity : ParamQuantity {

    float getDisplayValue() override {
        float v = getValue();
        if (!module) {
            return v;
        }

        return volumeToDb(v);
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
// DecibelsToAmplitude
//--------------------------------------------------------------

struct DecibelsToAmplitude {

  private:

    float curDb;
    float curAmp;
    bogaudio::dsp::SlewLimiter dbSlew;

  public:

    DecibelsToAmplitude() {
        curDb = kMinDb;
        curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        dbSlew.setLast(curDb);
    }

    void onSampleRateChange(float sampleRate) {
        dbSlew.setParams(sampleRate, 5.0f /* ms */, kMaxDb - kMinDb);
    }

    float next(float db) {

        float dbs = dbSlew.next(db);
        if (curDb != dbs) {
            curDb = dbs;

            // TODO perhaps we should use a lookup table here.
            curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        }
        return curAmp;
    }
};

//--------------------------------------------------------------
// VolumeAmplitude
//--------------------------------------------------------------

struct VolumeAmplitude {

  private:

    DecibelsToAmplitude volParam_dtoa;
    DecibelsToAmplitude volCvInput_dtoa;

  public:

    void onSampleRateChange(float sampleRate) {
        volParam_dtoa.onSampleRateChange(sampleRate);
        volCvInput_dtoa.onSampleRateChange(sampleRate);
    }

    float next(Param& volParam, bool muted, Input& volCvInput) {

        if (muted) {
            return volParam_dtoa.next(kMuteDb);
        }

        float amp = volParam_dtoa.next(volumeToDb(volParam.getValue()));

        if (volCvInput.isConnected()) {
            float cvDb = rescale(volCvInput.getVoltage(), 0.0f, 10.0f, kMinDb, kMaxDb);
            amp = amp * volCvInput_dtoa.next(cvDb);
        }

        return amp;
    }
};

//--------------------------------------------------------------
// VuLevel
//--------------------------------------------------------------

// VuLevel is adapted from github.com/bogaudio/BogaudioModules/src/VU.cpp
struct VuLevel {

  private:

    bogaudio::dsp::RootMeanSquare calcRms;

    bogaudio::dsp::RunningAverage calcPeak;
    bogaudio::dsp::SlewLimiter peakSlew;
    bool peakFalling = false;

    bogaudio::dsp::Timer maxPeakTimer;

  public:

    float rms = 0.0f;
    float peak = 0.0f;
    float maxPeak = 0.0f;

    void onSampleRateChange(float sampleRate) {

        calcRms.setSampleRate(sampleRate);
        calcRms.setSensitivity(1.0f);

        calcPeak.setSampleRate(sampleRate);
        calcPeak.setSensitivity(0.025f);
        peakSlew.setParams(sampleRate, 750.0f, 1.0f);

        maxPeakTimer.setParams(sampleRate, 1.0f);
    }

    void process(float sample) {

        // RMS
        rms = calcRms.next(sample) / 5.0f;

        // Peak
        float pa = calcPeak.next(fabsf(sample)) / 5.0f;

        if (pa < peak) {
            if (!peakFalling) {
                peakFalling = true;
                peakSlew.setLast(peak);
            }
            pa = peakSlew.next(pa);
        } else {
            peakFalling = false;
        }
        peak = pa;

        // Max Peak
        if ((peak > maxPeak) || !maxPeakTimer.next()) {
            maxPeak = peak;
            maxPeakTimer.reset();
        }
    }
};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

    float sum = 0.0f;
    VuLevel vuLevel;

    void onSampleRateChange(float sampleRate) {
        vuLevel.onSampleRateChange(sampleRate);
    }

    void process(Input& input, float amp) {

        sum = input.getVoltageSum() * amp;

        vuLevel.process(sum);
    }

    void disconnect() {
        sum = 0.0f;
        vuLevel.process(0.0f);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

struct StereoTrack {

  private:

    VolumeAmplitude volAmp;

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        volAmp.onSampleRateChange(sampleRate);
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(Input& leftInput, Input& rightInput, Param& volParam, bool muted, Input& volCvInput) {

        float amp = volAmp.next(volParam, muted, volCvInput);

        // left connected
        if (leftInput.isConnected()) {
            left.process(leftInput, amp);

            if (rightInput.isConnected()) {
                right.process(rightInput, amp); // stereo
            } else {
                right.process(leftInput, amp); // mono
            }
        }
        // left disconnected
        else {
            left.disconnect();

            if (rightInput.isConnected()) {
                right.process(rightInput, amp); // right only
            } else {
                right.disconnect(); // no inputs
            }
        }
    }
};

//--------------------------------------------------------------
// MonoMix
//--------------------------------------------------------------

struct MonoMix {

    float sum = 0.0f;
    VuLevel vuLevel;

    void onSampleRateChange(float sampleRate) {
        vuLevel.onSampleRateChange(sampleRate);
    }

    void process(float amp) {
        sum = sum * amp;
        vuLevel.process(sum);
    }
};

//--------------------------------------------------------------
// StereoMix
//--------------------------------------------------------------

struct StereoMix {

  private:

    VolumeAmplitude volAmp;

  public:

    MonoMix left;
    MonoMix right;

    void onSampleRateChange(float sampleRate) {
        volAmp.onSampleRateChange(sampleRate);
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(StereoTrack tracks[], int numTracks, Param& volParam, bool muted, Input& volCvInput) {

        left.sum = 0.0f;
        right.sum = 0.0f;
        for (int t = 0; t < numTracks; t++) {
            left.sum += tracks[t].left.sum;
            right.sum += tracks[t].right.sum;
        }

        float amp = volAmp.next(volParam, muted, volCvInput);

        left.process(amp);
        right.process(amp);
    }
};
