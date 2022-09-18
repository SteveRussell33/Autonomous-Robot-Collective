#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "../lib/bogaudio/dsp/signal.hpp"

inline float clamp(float x, float a = 0.f, float b = 1.f) {
	return std::fmax(std::fmin(x, b), a);
}

void dump(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(7);
    std::cout << v;
}

const float sampleRate = 48000.0f;

//--------------------------------------------------------------
// Amplitude
//--------------------------------------------------------------

static const float kMuteDb = -120.0f;
static const float kMinDb = -60.0f;
static const float kMaxDb = 12.0f;

// I think I adapted this from somewhere in the bogaudio codebase...
struct Amplitude {

  private:

    float curDb;
    float curAmp;
    bogaudio::dsp::SlewLimiter slew;

  public:

    Amplitude() {
        curDb = kMinDb;
        // TODO use a lookup table
        curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        slew.setLast(curDb);
    }

    void onSampleRateChange(float sampleRate) {
        slew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);
    }

    float next(float db) {

        float dbs = slew.next(db);
        if (curDb != dbs) {
            curDb = dbs;

            // TODO use a lookup table
            curAmp = bogaudio::dsp::decibelsToAmplitude(curDb);
        }
        return curAmp;
    }
};

void testAmplitude() {

    Amplitude faderAmp;

    faderAmp.onSampleRateChange(sampleRate);

    for (int i = 0; i < 100 * 10; i++) {
        float ampF = faderAmp.next(0.0f);

        std::cout << i << ": ";
        dump(ampF);
        std::cout << std::endl;
    }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

void testSlew() {

    bogaudio::dsp::SlewLimiter slew;
    slew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);

    slew.setLast(0);
    for (int i = 0; i < 100 * 10; i++) {
        dump(slew.next(kMuteDb));
        std::cout << std::endl;
    }
}

//void csv() {
//
//    std::cout << "T,A,B" << std::endl;
//
//    for (int i = 0; i < 10; i++) {
//        for (int j = 0; j < 100; j++) {
//
//            float t = i + j/100.0f;
//
//            float a = foo();
//            a = a * amplitude;
//
//            float b = bar();
//            b = b * amplitude;
//
//            dump(t);
//            std::cout << ",";
//            dump(a);
//            std::cout << ",";
//            dump(b);
//            std::cout << std::endl;
//        }
//    }
//}

//// don't forget to clamp at -1, 1
//void pan(float pan) {
//
//    float p = (pan + 1.0f) * 0.125f;
//    float left = std::cosf(2.0f * M_PI * p);
//    float right = std::sinf(2.0f * M_PI * p);
//
//}

//--------------------------------------------------------------
// Pan
//--------------------------------------------------------------

struct Pan {

  private:

    float curPan = 0.0f;

    bogaudio::dsp::SlewLimiter slew;

  public:

    float left = 0.7071068f;
    float right = 0.7071068f;

    Pan() {
        slew.setLast(0.0f);
    }

    void onSampleRateChange(float sampleRate) {
        slew.setParams(sampleRate, 5.0f, 2.0f);
    }

    void next(float pan) {

        pan = clamp(pan, -1.0f, 1.0f);

        float ps = slew.next(pan);
        if (curPan != ps) {
            curPan = ps;

            float p = (curPan + 1.0f) * 0.125f;

            // TODO use lookup tables
            left = std::cosf(2.0f * M_PI * p);
            right = std::sinf(2.0f * M_PI * p);
        }
    }
};

void testPan() {

    Pan pan;
    pan.onSampleRateChange(sampleRate);

    for (int i = 0; i < 100 * 10; i++) {
        pan.next(1.0f);

        std::cout << i << ": ";
        dump(pan.left);
        std::cout << ", ";
        dump(pan.right);
        std::cout << std::endl;
    }
}

void testDb() {

    int db = -120.0f;
    float amp = bogaudio::dsp::decibelsToAmplitude(db);

    dump(db);
    std::cout << ",";
    dump(amp);
    std::cout << std::endl;

    for (db = -60; db <= 12; db++) {
        amp = bogaudio::dsp::decibelsToAmplitude(db);

        dump(db);
        std::cout << ",";
        dump(amp);
        std::cout << std::endl;
    }
}

int main() {
    testPan();
}
