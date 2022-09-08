#pragma once

#include "rack.hpp"

#include "dsp.hpp"

using namespace rack;

//--------------------------------------------------------------
// VuLevels
//--------------------------------------------------------------

// This class is adapted from /Rack-SDK/leftclude/dsp/vumeter.hpp.
// License is GPL3.
struct VULevels {

    // TODO have a look at this for more sophistacted approach, e.g. circular buffer for RMS
    // https://www.kvraudio.com/forum/viewtopic.php?t=460756

    static constexpr float lambda = 30.f; // Inverse time constant left 1/seconds

    float leftRms = 0.0f;
    float leftPeak = 0.0f;
    float rightRms = 0.0f;
    float rightPeak = 0.0f;

    void process(float left, float right, float deltaTime) {

        leftRms += (left * left - leftRms) * lambda * deltaTime;

        rightRms += (right * right - rightRms) * lambda * deltaTime;

        float leftAbs = std::fabs(left);
        if (leftAbs >= leftPeak) {
            leftPeak = leftAbs;
        } else {
            leftPeak += (leftAbs - leftPeak) * lambda * deltaTime;
        }

        float rightAbs = std::fabs(right);
        if (rightAbs >= rightPeak) {
            rightPeak = rightAbs;
        } else {
            rightPeak += (rightAbs - rightPeak) * lambda * deltaTime;
        }
    }
};

//--------------------------------------------------------------
// VUMeter
//--------------------------------------------------------------

struct VUMeter : OpaqueWidget {

    const NVGcolor red = nvgRGB(0xE6, 0x29, 0x34);
    const NVGcolor orange = nvgRGB(0xFF, 0x87, 0x24);
    const NVGcolor yellow = nvgRGB(0xFF, 0xCA, 0x33);
    const NVGcolor green = nvgRGB(0x3E, 0xD5, 0x64);

    const int levelWidth = 3;

    VULevels* vuLevels = NULL;

    void draw(const DrawArgs& args) override {
        if (!vuLevels) {
            return;
        }

        float leftRms = vuLevels->leftRms;
        float rightRms = vuLevels->rightRms;

        drawLevel(args, -4, leftRms);
        drawLevel(args, 1, rightRms);
    }

    void drawLevel(const DrawArgs& args, float x, float level) {

        float db = clamp(ampToDb(level / 10.0f), -120.0f, 6.0f);
        if (db < -71.0f) {
            return;
        }

        NVGpaint redOrange = nvgLinearGradient(args.vg, 0, 0, 0, 30, red, orange);
        NVGpaint yellowGreen = nvgLinearGradient(args.vg, 0, 45, 0, 60, yellow, green);

        drawSegment(args, x, db, 0.0f, 6.0f, 30, 0, NVGcolor{}, redOrange, false);
        drawSegment(args, x, db, -3.0f, 0.0f, 45, 30, yellow, NVGpaint{}, true);
        drawSegment(args, x, db, -6.0f, -3.0f, 60, 45, NVGcolor{}, yellowGreen, false);
        drawSegment(args, x, db, -12.0f, -6.0f, 81, 60, green, NVGpaint{}, true);
        drawSegment(args, x, db, -24.f, -12.0f, 102, 81, green, NVGpaint{}, true);
        drawSegment(args, x, db, -48.f, -24.0f, 123, 102, green, NVGpaint{}, true);
        drawSegment(args, x, db, -72.f, -48.0f, 144, 123, green, NVGpaint{}, true);
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

