#pragma once

#include "track.hpp"

struct VuColors {
    NVGcolor orange;
    NVGcolor yellow;
    NVGcolor green;
};

struct VuMeter : OpaqueWidget {

  private:

    static const int kWidth = 5;

    VuLevel* vuLevel = NULL;

    // clang-format off
    VuColors fadedColors = {
        nvgRGBA(0xFF, 0x87, 0x24, 0xA0),
        nvgRGBA(0xFF, 0xCA, 0x33, 0xA0),
        nvgRGBA(0x3E, 0xD5, 0x64, 0xA0)};

    VuColors boldColors = {
        nvgRGB(0xFF, 0x87, 0x24), 
        nvgRGB(0xFF, 0xCA, 0x33), 
        nvgRGB(0x3E, 0xD5, 0x64)};
    // clang-format on

    inline float invLerp(float x, float low, float high) {
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

        NVGpaint yellowGreen =
            nvgLinearGradient(args.vg, 0.0f, 26.0f, 0.0f, 52.0f, colors.yellow, colors.green);

        // clang-format off
        drawSegment(args, x, db,  -3.0f,   0.0f,  13.0f, -1.0f, colors.orange, NVGpaint{},  true);
        drawSegment(args, x, db,  -6.0f,  -3.0f,  26.0f, 13.0f, colors.yellow, NVGpaint{},  true);
        drawSegment(args, x, db, -12.0f,  -6.0f,  52.0f, 26.0f, NVGcolor{},    yellowGreen, false);
        drawSegment(args, x, db, -18.0f, -12.0f,  65.0f, 52.0f, colors.green,  NVGpaint{},  true);
        drawSegment(args, x, db, -24.0f, -18.0f,  78.0f, 65.0f, colors.green,  NVGpaint{},  true);
        drawSegment(args, x, db, -36.0f, -24.0f,  91.0f, 78.0f, colors.green,  NVGpaint{},  true);
        drawSegment(args, x, db, -48.0f, -36.0f, 105.0f, 91.0f, colors.green,  NVGpaint{},  true);
        // clang-format on
    }

    NVGcolor getPeakColor(float db, VuColors colors) {
        // clang-format off
        if      (db >=  -3.0f) return colors.orange;
        else if (db >=  -6.0f) return colors.yellow;
        else if (db >= -12.0f) return nvgLerpRGBA(colors.green, colors.yellow, invLerp(db, -12.0f, -6.0f));
        else if (db >= -18.0f) return colors.green;
        else if (db >= -24.0f) return colors.green;
        else if (db >= -36.0f) return colors.green;
        else                   return colors.green;
        // clang-format on
    }

    float getPeakY(float db) {
        // clang-format off
        if      (db >=  -3.0f) return rescale(db,  -3.0f,   0.0f,  13.0f, -1.0f);
        else if (db >=  -6.0f) return rescale(db,  -6.0f,  -3.0f,  26.0f, 13.0f);
        else if (db >= -12.0f) return rescale(db, -12.0f,  -6.0f,  52.0f, 26.0f);
        else if (db >= -18.0f) return rescale(db, -18.0f, -12.0f,  65.0f, 52.0f);
        else if (db >= -24.0f) return rescale(db, -24.0f, -18.0f,  78.0f, 65.0f);
        else if (db >= -36.0f) return rescale(db, -36.0f, -24.0f,  91.0f, 78.0f);
        else                   return rescale(db, -48.0f, -36.0f, 105.0f, 91.0f);
        // clang-format on
    }

    void drawMaxPeak(const DrawArgs& args, float x, float maxPeak, VuColors colors) {

        float db = clamp(bogaudio::dsp::amplitudeToDecibels(maxPeak), -48.0f, 0.0f);
        if (db < -45.0f) {
            return;
        }

        drawRect(args, x, getPeakY(db), kWidth, 1, getPeakColor(db, colors), NVGpaint{}, true);
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
