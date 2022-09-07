#pragma once

#include "rack.hpp"

using namespace rack;

struct VuMeter : OpaqueWidget {

    const NVGcolor green = nvgRGB(0x3E, 0xD5, 0x64);

	void draw(const DrawArgs& args) override {

        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, 3, 144);
        nvgFillColor(args.vg, green);
        nvgFill(args.vg);

        nvgBeginPath(args.vg);
        nvgRect(args.vg, 5, 0, 3, 144);
        nvgFillColor(args.vg, green);
        nvgFill(args.vg);

        //nvgBeginPath(args.vg);
        //nvgRect(args.vg, 5, 0, 8, 144);
        //nvgFillColor(args.vg, green);
        //nvgFill(args.vg);
    }
};
