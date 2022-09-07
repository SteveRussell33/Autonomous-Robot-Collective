#pragma once

#include "rack.hpp"

using namespace rack;

struct Meter : OpaqueWidget {

    const NVGcolor bg = nvgRGB(0x34, 0x34, 0x34);
    const NVGcolor green = nvgRGB(0x3E, 0xD7, 0x65);

	void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, 8, 144);
        nvgFillColor(args.vg, bg);
        nvgFill(args.vg);

        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, 3, 144);
        nvgFillColor(args.vg, green);
        nvgFill(args.vg);

        //nvgBeginPath(args.vg);
        //nvgRect(args.vg, 5, 0, 8, 144);
        //nvgFillColor(args.vg, green);
        //nvgFill(args.vg);
    }
};
