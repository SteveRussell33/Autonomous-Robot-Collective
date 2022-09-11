#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

using namespace rack;

const float kMinDb = -72.0f;
const float kMaxDb = 6.0f;

//--------------------------------------------------------------
// Fader
//--------------------------------------------------------------

// These values were all empiricially determined by moving the fader handle
// around so that it lined up with the various tick marks.
const float kFaderDbPlus6 = 1.0f;
const float kFaderDbPlus3 = 0.896f;
const float kFaderDbZero = 0.79f;
const float kFaderDbMinus3 = 0.686f;
const float kFaderDbMinus6 = 0.58f;
const float kFaderDbMinus12 = 0.432f;
const float kFaderDbMinus24 = 0.286f;
const float kFaderDbMinus48 = 0.136f;
const float kFaderDbMinus72 = 0.0f;

float faderToDb(float v) {
    if (v >= kFaderDbMinus6) {
        return rescale(v, kFaderDbMinus6, kFaderDbPlus6, -6.0f, 6.0f);
    } else if (v >= kFaderDbMinus12) {
        return rescale(v, kFaderDbMinus12, kFaderDbMinus6, -12.0f, -6.0f);
    } else if (v >= kFaderDbMinus24) {
        return rescale(v, kFaderDbMinus24, kFaderDbMinus12, -24.0f, -12.0f);
    } else if (v >= kFaderDbMinus48) {
        return rescale(v, kFaderDbMinus48, kFaderDbMinus24, -48.0f, -24.0f);
    } else {
        return rescale(v, kFaderDbMinus72, kFaderDbMinus48, -72.0f, -48.0f);
    }
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

        v = clamp(v, -72.0f, 6.0f);

        if (v >= -6.0f) {
            v = rescale(v, -6.0f, 6.0f, kFaderDbMinus6, kFaderDbPlus6);
        } else if (v >= -12.0f) {
            v = rescale(v, -12.0f, -6.0f, kFaderDbMinus12, kFaderDbMinus6);
        } else if (v >= -24.0f) {
            v = rescale(v, -24.0f, -12.0f, kFaderDbMinus24, kFaderDbMinus12);
        } else if (v >= -48.0f) {
            v = rescale(v, -48.0f, -24.0f, kFaderDbMinus48, kFaderDbMinus24);
        } else {
            v = rescale(v, -72.0f, -48.0f, kFaderDbMinus72, kFaderDbMinus48);
        }

        setValue(v);
    }
};

//--------------------------------------------------------------
// Amplitude
//--------------------------------------------------------------

struct Amplitude {

private:

    float curDb;
    float curAmp;
    bogaudio::dsp::SlewLimiter dbSlew;

public:

    Amplitude() {
        curDb = kMinDb;
        curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        dbSlew.setLast(curDb);
    }

    void sampleRateChange(float sampleRate) {
        dbSlew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
    }

    // Convert db to amplitude
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
// Track
//--------------------------------------------------------------

struct Track {

  private:

    Param* faderParam;
    Param* muteParam;
    Input* levelInput;

    Amplitude faderAmplitude;
    Amplitude levelAmplitude;

  public:

    void init(Param* faderParam_, Param* muteParam_, Input* levelParam_) {
        faderParam = faderParam_;
        muteParam = muteParam_;
        levelInput = levelParam_;
    }

    void sampleRateChange(float sampleRate) {
        faderAmplitude.sampleRateChange(sampleRate);
        levelAmplitude.sampleRateChange(sampleRate);
    }

    float nextAmplitude() {

        bool muted = muteParam->getValue() > 0.5f;
        if (muted) {
            return faderAmplitude.next(kMinDb);
        }

        float faderDb = faderToDb(faderParam->getValue());
        float faderAmp = faderAmplitude.next(faderDb);

        if (levelInput->isConnected()) {

            // Scale the level input voltage exponentially from [0V, 10V] to [-72dB, +6dB].  
            // Note that levelInput is monophonic.
            float levelDb = rescale(levelInput->getVoltage(), 0.0f, 10.0f, kMinDb, kMaxDb);
            float levelAmp = levelAmplitude.next(levelDb);

            // Use the fader amplitude as an exponential CV control 
            // on the level amplitude.
            return levelAmp * faderAmp;
        }
        else {
            return faderAmp;
        }
    }
};

//--------------------------------------------------------------
// MonoLevels
//--------------------------------------------------------------

// MonoLevels is adapted from github.com/bogaudio/BogaudioModules/src/VU.cpp
struct MonoLevels {

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

    void sampleRateChange(float sampleRate) {

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
// StereoLevels
//--------------------------------------------------------------

struct StereoLevels {

    MonoLevels left;
    MonoLevels right;

    void sampleRateChange(float sampleRate) {
        left.sampleRateChange(sampleRate);
        right.sampleRateChange(sampleRate);
    }
};

//--------------------------------------------------------------
// VUMeter
//--------------------------------------------------------------

struct VUColors {
    NVGcolor red;
    NVGcolor orange;
    NVGcolor yellow;
    NVGcolor green;
};

struct VUMeter : OpaqueWidget {

  private:

    static const int kLevelWidth = 3;

    VUColors fadedColors = {
        nvgRGBA(0xE6, 0x29, 0x34, 0xA0),
        nvgRGBA(0xFF, 0x87, 0x24, 0xA0),
        nvgRGBA(0xFF, 0xCA, 0x33, 0xA0),
        nvgRGBA(0x3E, 0xD5, 0x64, 0xA0)};

    VUColors boldColors = {
        nvgRGB(0xE6, 0x29, 0x34),
        nvgRGB(0xFF, 0x87, 0x24),
        nvgRGB(0xFF, 0xCA, 0x33),
        nvgRGB(0x3E, 0xD5, 0x64)};

    void drawLevel(const DrawArgs& args, float x, float level, VUColors colors) {

        float dB = clamp(bogaudio::dsp::amplitudeToDecibels(level), kMinDb, 6.0f);
        if (dB < kMinDb + 1.0f) {
            return;
        }

        NVGpaint redOrange = nvgLinearGradient(args.vg, 0, 15, 0, 30, colors.red, colors.orange);
        NVGpaint yellowGreen =
            nvgLinearGradient(args.vg, 0, 45, 0, 60, colors.yellow, colors.green);

        drawSegment(args, x, dB, 3.0f, 6.0f, 15, 0, colors.red, NVGpaint{}, true);
        drawSegment(args, x, dB, 0.0f, 3.0f, 30, 15, NVGcolor{}, redOrange, false);
        drawSegment(args, x, dB, -3.0f, 0.0f, 45, 30, colors.yellow, NVGpaint{}, true);
        drawSegment(args, x, dB, -6.0f, -3.0f, 60, 45, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, dB, -12.0f, -6.0f, 81, 60, colors.green, NVGpaint{}, true);
        drawSegment(args, x, dB, -24.f, -12.0f, 102, 81, colors.green, NVGpaint{}, true);
        drawSegment(args, x, dB, -48.f, -24.0f, 123, 102, colors.green, NVGpaint{}, true);
        drawSegment(args, x, dB, -72.f, -48.0f, 144, 123, colors.green, NVGpaint{}, true);
    }

    void drawSegment(
        const DrawArgs& args,
        float x,
        float dB,
        float lowDb,
        float highDb,
        float bottom,
        float top,
        NVGcolor color,
        NVGpaint gradient,
        bool isColor) {

        if (dB < lowDb) {
            return;
        }

        float y = (dB > highDb) ? top : rescale(dB, lowDb, highDb, bottom, top);
        float height = bottom - y;
        drawRect(args, x, y, kLevelWidth, height, color, gradient, isColor);
    }

    void drawMaxPeak(const DrawArgs& args, float x, float maxPeak, VUColors colors) {

        float dB = clamp(bogaudio::dsp::amplitudeToDecibels(maxPeak), kMinDb, 6.0f);
        if (dB < kMinDb + 1.0f) {
            return;
        }

        float y = 0.0f;
        NVGcolor col;

        if (dB < -48.0f) {
            y = rescale(dB, -72.f, -48.0f, 144, 123);
            col = colors.green;
        } else if (dB < -24.0f) {
            y = rescale(dB, -48.f, -24.0f, 123, 102);
            col = colors.green;
        } else if (dB < -12.0f) {
            y = rescale(dB, -24.f, -12.0f, 102, 81);
            col = colors.green;
        } else if (dB < -6.0f) {
            y = rescale(dB, -12.0f, -6.0f, 81, 60);
            col = colors.green;
        } else if (dB < -3.0f) {
            y = rescale(dB, -6.0f, -3.0f, 60, 45);
            col = nvgLerpRGBA(colors.green, colors.yellow, linearDistance(dB, -6.0f, -3.0f));
        } else if (dB < 0.0f) {
            y = rescale(dB, -3.0f, 0.0f, 45, 30);
            col = colors.yellow;
        } else if (dB < 3.0f) {
            y = rescale(dB, 0.0f, 3.0f, 30, 15);
            col = nvgLerpRGBA(colors.yellow, colors.red, linearDistance(dB, 0.0f, 3.0f));
        } else {
            y = rescale(dB, 3.0f, 6.0f, 15, 0);
            col = colors.red;
        }

        drawRect(args, x, y, kLevelWidth, 1, col, NVGpaint{}, true);
    }

    inline float linearDistance(float x, float low, float high) {
        return (x - low) / (high - low);
    }

    void drawRect(
        const DrawArgs& args,
        float x,
        float y,
        float w,
        float h,
        NVGcolor color,
        NVGpaint gradient,
        bool isColor) {

        nvgBeginPath(args.vg);
        nvgRect(args.vg, x, y, w, h);
        if (isColor) {
            nvgFillColor(args.vg, color);
        } else {
            nvgFillPaint(args.vg, gradient);
        }
        nvgFill(args.vg);
    }

  public:

    StereoLevels* levels = NULL;

    void draw(const DrawArgs& args) override {
        if (!levels) {
            return;
        }

        drawLevel(args, -4, levels->left.peak, fadedColors);
        drawLevel(args, 1, levels->right.peak, fadedColors);

        drawMaxPeak(args, -4, levels->left.maxPeak, boldColors);
        drawMaxPeak(args, 1, levels->right.maxPeak, boldColors);

        drawLevel(args, -4, levels->left.rms, boldColors);
        drawLevel(args, 1, levels->right.rms, boldColors);
    }
};
