//#pragma once
//
//#include "rack.hpp"
//
//#include "dsp.hpp"
//
// using namespace rack;
//
//
// struct VUMeter : OpaqueWidget {
//
//    const float height = 144.0f;
//    const NVGcolor green = nvgRGB(0x3E, 0xD5, 0x64);
//
//    // TODO make a base class that we can get VULevels from
//    GAIN* module = NULL;
//
//	VUMeter(GAIN* module_) : module(module_) {}
//
//	void draw(const DrawArgs& args) override {
//        float leftRms = 0.0;
//		if (module) {
//            // TODO make a base class that we can get VULevels from
//            leftRms = module->levels.leftRms;
//        }
//
//        float db = clamp(ampToDb(leftRms/10.0f), -120.0f, 6.0f);
//        if (db < -119.0f) {
//            return;
//        }
//        float y = rescale(db, -120.0f, 6.0f, height, 0.0f);
//
//        nvgBeginPath(args.vg);
//        nvgRect(args.vg, 0, y, 3, height - y);
//        nvgFillColor(args.vg, green);
//        nvgFill(args.vg);
//
//        //nvgBeginPath(args.vg);
//        //nvgRect(args.vg, 5, 0, 3, 144);
//        //nvgFillColor(args.vg, green);
//        //nvgFill(args.vg);
//
//        //nvgBeginPath(args.vg);
//        //nvgRect(args.vg, 5, 0, 8, 144);
//        //nvgFillColor(args.vg, green);
//        //nvgFill(args.vg);
//    }
//};
