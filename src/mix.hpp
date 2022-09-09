#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/signal.hpp"
#include "bogaudio/dsp/filters/utility.hpp"

using namespace rack;

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

// This class is adapted from github.com/bogaudio/BogaudioModules/src/VU.cpp
struct MonoTrack {

private:

	bogaudio::dsp::RootMeanSquare rms;

    bogaudio::dsp::RunningAverage peakAvg;
	bogaudio::dsp::SlewLimiter peakSlew;
	bool peakFalling = false;

public:

    float rmsLevel = 0.0f;
    float peakLevel = 0.0f;

    void sampleRateChange(float sampleRate) {

        rms.setSampleRate(sampleRate);
		rms.setSensitivity(1.0f);

        peakAvg.setSampleRate(sampleRate);
		peakAvg.setSensitivity(0.025f);
        peakSlew.setParams(sampleRate, 750.0f, 1.0f);
    }

	void process(float sample) {

        // RMS
        rmsLevel = rms.next(sample) / 5.0f;

        // Peak
        float peak = peakAvg.next(fabsf(sample)) / 5.0f;

        if (peak < peakLevel) {
            if (!peakFalling) {
                peakFalling = true;
                peakSlew._last = peakLevel;
            }
            peak = peakSlew.next(peak);
        }
        else {
            peakFalling = false;
        }
        peakLevel = peak;
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

    VUColors peakColors = {
        nvgRGBA(0xE6, 0x29, 0x34, 0xA8),
        nvgRGBA(0xFF, 0x87, 0x24, 0xA8),
        nvgRGBA(0xFF, 0xCA, 0x33, 0xA8),
        nvgRGBA(0x3E, 0xD5, 0x64, 0xA8)};

    VUColors rmsColors = {
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

        drawLevel(args, -4, track->left.peakLevel, peakColors);
        drawLevel(args, 1, track->right.peakLevel, peakColors);

        drawLevel(args, -4, track->left.rmsLevel, rmsColors);
        drawLevel(args, 1, track->right.rmsLevel, rmsColors);
    }

    void drawLevel(const DrawArgs& args, float x, float level, VUColors col) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(level), -120.0f, 6.0f);
        if (db < -71.0f) {
            return;
        }

        NVGpaint redOrange = nvgLinearGradient(args.vg, 0, 15, 0, 30, col.red, col.orange);
        NVGpaint yellowGreen = nvgLinearGradient(args.vg, 0, 45, 0, 60, col.yellow, col.green);

        drawSegment(args, x, db, 3.0f, 6.0f, 15, 0, col.red, NVGpaint{}, true);
        drawSegment(args, x, db, 0.0f, 3.0f, 30, 15, NVGcolor{}, redOrange, false);
        drawSegment(args, x, db, -3.0f, 0.0f, 45, 30, col.yellow, NVGpaint{}, true);
        drawSegment(args, x, db, -6.0f, -3.0f, 60, 45, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, db, -12.0f, -6.0f, 81, 60, col.green, NVGpaint{}, true);
        drawSegment(args, x, db, -24.f, -12.0f, 102, 81, col.green, NVGpaint{}, true);
        drawSegment(args, x, db, -48.f, -24.0f, 123, 102, col.green, NVGpaint{}, true);
        drawSegment(args, x, db, -72.f, -48.0f, 144, 123, col.green, NVGpaint{}, true);
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
