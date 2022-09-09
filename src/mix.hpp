#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

using namespace rack;

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

// This class is adapted from github.com/bogaudio/BogaudioModules/src/VU.cpp
struct MonoTrack {

  private:

    bogaudio::dsp::RootMeanSquare calcRms;

    bogaudio::dsp::RunningAverage peakAvg;
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

        peakAvg.setSampleRate(sampleRate);
        peakAvg.setSensitivity(0.025f);
        peakSlew.setParams(sampleRate, 750.0f, 1.0f);

        maxPeakTimer.setParams(sampleRate, 1.0f);
    }

    void process(float sample) {

        // RMS
        rms = calcRms.next(sample) / 5.0f;

        // Peak
        float pa = peakAvg.next(fabsf(sample)) / 5.0f;

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
// StereoTrack
//--------------------------------------------------------------

struct StereoTrack {

    MonoTrack left;
    MonoTrack right;

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

    const int levelWidth = 3;

    // This is set in the constructor of the parent ModuleWidget.
    // It could still be NULL though.
    StereoTrack* track = NULL;

    void draw(const DrawArgs& args) override {
        if (!track) {
            return;
        }

        drawLevel(args, -4, track->left.peak, fadedColors);
        drawLevel(args, 1, track->right.peak, fadedColors);

        drawMaxPeak(args, -4, track->left.maxPeak, boldColors);
        drawMaxPeak(args, 1, track->right.maxPeak, boldColors);

        drawLevel(args, -4, track->left.rms, boldColors);
        drawLevel(args, 1, track->right.rms, boldColors);
    }

    void drawLevel(const DrawArgs& args, float x, float level, VUColors colors) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(level), -120.0f, 6.0f);
        if (db < -71.0f) {
            return;
        }

        NVGpaint redOrange = nvgLinearGradient(args.vg, 0, 15, 0, 30, colors.red, colors.orange);
        NVGpaint yellowGreen =
            nvgLinearGradient(args.vg, 0, 45, 0, 60, colors.yellow, colors.green);

        drawSegment(args, x, db, 3.0f, 6.0f, 15, 0, colors.red, NVGpaint{}, true);
        drawSegment(args, x, db, 0.0f, 3.0f, 30, 15, NVGcolor{}, redOrange, false);
        drawSegment(args, x, db, -3.0f, 0.0f, 45, 30, colors.yellow, NVGpaint{}, true);
        drawSegment(args, x, db, -6.0f, -3.0f, 60, 45, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, db, -12.0f, -6.0f, 81, 60, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -24.f, -12.0f, 102, 81, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -48.f, -24.0f, 123, 102, colors.green, NVGpaint{}, true);
        drawSegment(args, x, db, -72.f, -48.0f, 144, 123, colors.green, NVGpaint{}, true);
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
        drawRect(args, x, y, levelWidth, height, color, gradient, isColor);
    }

    void drawMaxPeak(const DrawArgs& args, float x, float maxPeak, VUColors colors) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(maxPeak), -120.0f, 6.0f);
        if (db < -71.0f) {
            return;
        }

        float y = 0.0f;
        NVGcolor col;

        if (db < -48.0f) {
            y = rescale(db, -72.f, -48.0f, 144, 123);
            col = colors.green;
        } else if (db < -24.0f) {
            y = rescale(db, -48.f, -24.0f, 123, 102);
            col = colors.green;
        } else if (db < -12.0f) {
            y = rescale(db, -24.f, -12.0f, 102, 81);
            col = colors.green;
        } else if (db < -6.0f) {
            y = rescale(db, -12.0f, -6.0f, 81, 60);
            col = colors.green;
        } else if (db < -3.0f) {
            y = rescale(db, -6.0f, -3.0f, 60, 45);
            col = nvgLerpRGBA(colors.green, colors.yellow, dist(db, -6.0f, -3.0f));
        } else if (db < 0.0f) {
            y = rescale(db, -3.0f, 0.0f, 45, 30);
            col = colors.yellow;
        } else if (db < 3.0f) {
            y = rescale(db, 0.0f, 3.0f, 30, 15);
            col = nvgLerpRGBA(colors.yellow, colors.red, dist(db, 0.0f, 3.0f));
        } else {
            y = rescale(db, 3.0f, 6.0f, 15, 0);
            col = colors.red;
        }

        drawRect(args, x, y, levelWidth, 1, col, NVGpaint{}, true);
    }

    float dist(float x, float low, float high) { return (x - low) / (high - low); }

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
};
