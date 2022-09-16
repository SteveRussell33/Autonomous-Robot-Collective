#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

using namespace rack;

static const float kMuteDb = -120.0f;

static const float kMinDb = -60.0f;
static const float kMaxDb = 12.0f;

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

            // TODO lookup table
            curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        }
        return curAmp;
    }
};

//--------------------------------------------------------------
// VolumeControl
//--------------------------------------------------------------

struct VolumeControl {

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

////--------------------------------------------------------------
//// PanControl
////--------------------------------------------------------------
//
//struct PanControl {
//
//  private:
//
//    static constexpr float kCenter = 0.7071068f;
//    static constexpr float kTwoPi = 2.0f * M_PI;
//
//    float curPan = 0.0f;
//    bogaudio::dsp::SlewLimiter panSlew;
//
//  public:
//
//    float left = kCenter;
//    float right = kCenter;
//
//    void onSampleRateChange(float sampleRate) {
//        panSlew.setParams(sampleRate, 5.0f /* ms */, 1.0f);
//        panSlew.setLast(kCenter);
//    }
//
//    void next(float pan) {
//
//        float ps = panSlew.next(clamp(pan, -1.0f, 1.0f));
//        if (curPan != ps) {
//            curPan = ps;
//
//            // TODO lookup table
//            float p = (curPan + 1.0f) * 0.125f;
//            left = std::cosf(kTwoPi * p);
//            right = std::sinf(kTwoPi * p); // or cosf() with 0.75f....
//        }
//    }
//};

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

    // TODO: volume, pan per channel 
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

    VolumeControl volControl;

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        volControl.onSampleRateChange(sampleRate);
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(
        Input& leftInput,
        Input& rightInput,
        Param& volParam,
        bool muted,
        Input& volCvInput
        /*Param& panParam,
        Input& panCvInput*/) {

        float amp = volControl.next(volParam, muted, volCvInput);

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

    VolumeControl volControl;

  public:

    MonoMix left;
    MonoMix right;

    void onSampleRateChange(float sampleRate) {
        volControl.onSampleRateChange(sampleRate);
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

        float amp = volControl.next(volParam, muted, volCvInput);

        left.process(amp);
        right.process(amp);
    }
};
