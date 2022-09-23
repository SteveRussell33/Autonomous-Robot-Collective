//#include <cmath>
//#include <iomanip>
//#include <iostream>
//#include <sstream>
//#include <vector>
//
//#include "../lib/bogaudio/dsp/signal.hpp"
//
// inline float clamp(float x, float a = 0.f, float b = 1.f) {
//	return std::fmax(std::fmin(x, b), a);
//}
//
// void dump(float v) {
//    std::cout << std::fixed;
//    std::cout << std::setprecision(7);
//    std::cout << v;
//}
//
// const float sampleRate = 48000.0f;
//
////--------------------------------------------------------------
//// Amplitude
////--------------------------------------------------------------
//
// static const float kMinDb = -60.0f;
// static const float kMaxDb = 12.0f;
//
// struct Amplitude {
//
//  private:
//
//    float curDb;
//    float curAmp;
//    bogaudio::dsp::SlewLimiter slew;
//
//  public:
//
//    Amplitude() {
//        curDb = kMinDb;
//        curAmp = 0.0f;
//        slew.setLast(curDb);
//    }
//
//    void onSampleRateChange(float sampleRate) {
//        slew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
//    }
//
//    float next(float db) {
//
//        float dbs = slew.next(db);
//        if (curDb != dbs) {
//            curDb = dbs;
//
//            // TODO use a lookup table
//            curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
//
//            if (curAmp < 0.0011220 /* -59 dB */) {
//                curAmp = 0.0f;
//            }
//        }
//        return curAmp;
//    }
//};
//
// void testAmplitude() {
//
//    Amplitude faderAmp;
//
//    faderAmp.onSampleRateChange(sampleRate);
//
//    for (int i = 0; i < 500; i++) {
//        float ampF = faderAmp.next(kMaxDb);
//        std::cout << i << ": ";
//        dump(ampF);
//        std::cout << std::endl;
//    }
//
//    for (int i = 0; i < 500; i++) {
//        float ampF = faderAmp.next(kMinDb);
//        std::cout << i << ": ";
//        dump(ampF);
//        std::cout << std::endl;
//    }
//}
//
////--------------------------------------------------------------
////--------------------------------------------------------------
////--------------------------------------------------------------
//
// void testSlew() {
//
//    bogaudio::dsp::SlewLimiter slew;
//    slew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
//
//    slew.setLast(0);
//    for (int i = 0; i < 100 * 10; i++) {
//        dump(slew.next(kMinDb));
//        std::cout << std::endl;
//    }
//}
//
////void csv() {
////
////    std::cout << "T,A,B" << std::endl;
////
////    for (int i = 0; i < 10; i++) {
////        for (int j = 0; j < 100; j++) {
////
////            float t = i + j/100.0f;
////
////            float a = foo();
////            a = a * amplitude;
////
////            float b = bar();
////            b = b * amplitude;
////
////            dump(t);
////            std::cout << ",";
////            dump(a);
////            std::cout << ",";
////            dump(b);
////            std::cout << std::endl;
////        }
////    }
////}
//
////// don't forget to clamp at -1, 1
////void pan(float pan) {
////
////    float p = (pan + 1.0f) * 0.125f;
////    float left = std::cosf(2.0f * M_PI * p);
////    float right = std::sinf(2.0f * M_PI * p);
////
////}
//
////--------------------------------------------------------------
//// Pan
////--------------------------------------------------------------
//
// struct Pan {
//
//  private:
//
//    float curPan = 0.0f;
//
//    bogaudio::dsp::SlewLimiter slew;
//
//  public:
//
//    float left = 0.7071068f;
//    float right = 0.7071068f;
//
//    Pan() {
//        slew.setLast(0.0f);
//    }
//
//    void onSampleRateChange(float sampleRate) {
//        slew.setParams(sampleRate, 5.0f, 2.0f);
//    }
//
//    void next(float pan) {
//
//        pan = clamp(pan, -1.0f, 1.0f);
//
//        float ps = slew.next(pan);
//        if (curPan != ps) {
//            curPan = ps;
//
//            float p = (curPan + 1.0f) * 0.125f;
//
//            // TODO use lookup tables
//            left = std::cosf(2.0f * M_PI * p);
//            right = std::sinf(2.0f * M_PI * p);
//        }
//    }
//};
//
// void testPan() {
//
//    Pan pan;
//    pan.onSampleRateChange(sampleRate);
//
//    for (int i = 0; i < 100 * 10; i++) {
//        pan.next(1.0f);
//
//        std::cout << i << ": ";
//        dump(pan.left);
//        std::cout << ", ";
//        dump(pan.right);
//        std::cout << std::endl;
//    }
//}
//
// void testDb() {
//
//    for (int db = -60; db <= -50; db++) {
//        float amp = bogaudio::dsp::decibelsToAmplitude(db);
//        if (amp < 0.0011220 /* -59 dB */) {
//            amp = 0.0f;
//        }
//
//        dump(db);
//        std::cout << ",";
//        dump(amp);
//        std::cout << std::endl;
//    }
//}
//
// int main() {
//    testAmplitude();
//}
//

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

#include <iomanip>
#include <iostream>
#include <cmath>

#include "../lib/bogaudio/dsp/signal.hpp"
#include "../src/dsp/arc_table.hpp"

//--------------------------------------------------------------

 void dump(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(7);
    std::cout << v;
}

constexpr float factorial(int n) {
    return (n < 2) ? 1 : (n * factorial(n-1));
}

constexpr float rescale(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

constexpr float dbtoa(int n) {
    return std::powf(10.0f, rescale(n, 0, 10, -60, 20) * 0.05f)
}

//-60.0f + (x - 0.0f) / (xMax - 0.0f) * (yMax - -60.0f)
//
void testTable() {
    constexpr auto lut = arc::dsp::LUT<10>(dbtoa);
    for (int i = 0; i < 10; i++) {
        dump(lut[i]);
        std::cout << std::endl;
    }
}

int main() {
    testTable();
}
