#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

using namespace rack;

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

    float voltageSum = 0.0f;
    VuLevel vuLevel;

    void onSampleRateChange(float sampleRate) {
        vuLevel.onSampleRateChange(sampleRate);
    }

    void process(Input& input) {

        // process each channel
        voltageSum = 0.0f;
        int channels = std::max(input.getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = input.getPolyVoltage(ch);

            float out = in;

            voltageSum += out;
        }

        // done
        vuLevel.process(voltageSum);
    }

    void disconnect() {
        voltageSum = 0.0f;
        vuLevel.process(0.0f);
    }
};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct StereoTrack {

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(Input& leftInput, Input& rightInput) {

        if (leftInput.isConnected()) {
            left.process(leftInput);
        } else {
            left.disconnect();
        }

        if (rightInput.isConnected()) {
            right.process(rightInput);
        } else {
            right.disconnect();
        }
    }
};

//--------------------------------------------------------------
// VuMeter
//--------------------------------------------------------------

struct VuColors {
    NVGcolor orange;
    NVGcolor yellow;
    NVGcolor green;
};

struct VuMeter : OpaqueWidget {

  private:

    static const int kWidth = 5;

    VuLevel* vuLevel = NULL;

    VuColors fadedColors = {
        nvgRGBA(0xFF, 0x87, 0x24, 0xA0),
        nvgRGBA(0xFF, 0xCA, 0x33, 0xA0),
        nvgRGBA(0x3E, 0xD5, 0x64, 0xA0)};

    VuColors boldColors = {
        nvgRGB(0xFF, 0x87, 0x24), 
        nvgRGB(0xFF, 0xCA, 0x33), 
        nvgRGB(0x3E, 0xD5, 0x64)};

    inline float invLerp(float x, float low, float high) {
        return (x - low) / (high - low);
    }

    void drawRect(
        const DrawArgs& args, float x, float y, float w, float h, NVGcolor color, NVGpaint gradient, bool isColor) {

        nvgBeginPath(args.vg);
        nvgRect(args.vg, x, y, w, h);
        if (isColor) {
            nvgFillColor(args.vg, color);
        } else {
            nvgFillPaint(args.vg, gradient);
        }
        nvgFill(args.vg);
    }

    void drawSegment(
        const DrawArgs& args,
        float x,
        float db,
        float lowDb,
        float highDb,
        float bottom,
        float top,
        NVGcolor color,
        NVGpaint gradient,
        bool isColor) {

        if (db < lowDb) {
            return;
        }

        float y = (db > highDb) ? top : rescale(db, lowDb, highDb, bottom, top);
        float height = bottom - y;
        drawRect(args, x, y, kWidth, height, color, gradient, isColor);
    }

    void drawLevel(const DrawArgs& args, float x, float level, VuColors colors) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(level), -48.0f, 0.0f);
        if (db < -45.0f) {
            return;
        }

        NVGpaint yellowGreen = nvgLinearGradient(args.vg, 0, 26, 0, 39, colors.yellow, colors.green);

        drawSegment(args, x, db,  -3,   0,     13,  -1, colors.orange, NVGpaint{}, true);
        drawSegment(args, x, db,  -6,  -3,     26,     13, colors.yellow, NVGpaint{}, true);
        drawSegment(args, x, db,  -9,  -6,     39,     26, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, db, -12,  -9,     52,     39, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -18, -12,     65,     52, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -24, -18,     78,     65, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -36, -24,     91,     78, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -48, -36, 105,     91, colors.green, NVGpaint{}, true);
    }

    void drawMaxPeak(const DrawArgs& args, float x, float maxPeak, VuColors colors) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(maxPeak), -48.0f, 0.0f);
        if (db < -45.0f) {
            return;
        }

        float y = 0;
        NVGcolor col;

        if (db < -48) {
            y = rescale(db, -48, -36, 105,  91);
            col = colors.green;
        } else if (db < -36) {
            y = rescale(db, -36, -24,  91,  78);
            col = colors.green;
        } else if (db < -24) {
            y = rescale(db, -24, -18,  78,  65);
            col = colors.green;
        } else if (db < -18) {
            y = rescale(db, -18, -12,  65,  52);
            col = colors.green;
        } else if (db < -12) {
            y = rescale(db, -12,  -9,  52,  39);
            col = nvgLerpRGBA(colors.green, colors.yellow, invLerp(db, -0, -3));
        } else if (db < -9) {
            y = rescale(db, -9,  -6,  39,  26);
            col = colors.yellow;
        } else if (db < -6) {
            y = rescale(db, -6,  -3,  26,  13);
            col = nvgLerpRGBA(colors.yellow, colors.orange, invLerp(db, 0, 3));
        } else {
            y = rescale(db, -3,   0,  13,   -1);
            col = colors.orange;
        }

        drawRect(args, x, y, kWidth, 1, col, NVGpaint{}, true);
    }

  public:

    VuMeter(VuLevel* vuLevel_) : vuLevel(vuLevel_) {
    }

    void draw(const DrawArgs& args) override {
        if (!vuLevel) {
            return;
        }

        drawLevel(args, 0, vuLevel->peak, fadedColors);
        drawMaxPeak(args, 0, vuLevel->maxPeak, boldColors);
        drawLevel(args, 0, vuLevel->rms, boldColors);
    }
};
