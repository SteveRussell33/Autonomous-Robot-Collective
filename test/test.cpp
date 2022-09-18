#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

//include "../../lib/bogaudio/dsp/signal.hpp"

void dump(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(7);
    std::cout << v;
}

// "amplitude" is 0-whatever here, with 1 (=0db) meaning unity gain.
inline float decibelsToAmplitude(float db) {
	return powf(10.0f, db * 0.05f);
}

inline float amplitudeToDecibels(float amplitude) {
	if (amplitude < 0.000001f) {
		return -120.0f;
	}
	return 20.0f * log10f(amplitude);
}

//void slew() {
//
//    const float sampleRate = 48000.0f;
//    const float slewTime = 5.0f; // millis
//
//    bogaudio::dsp::SlewLimiter slew;
//    slew.setParams(sampleRate, slewTime, 1.0f);
//
//    slew.setLast(0.9);
//    for (int i = 0; i < 100; i++) {
//        dump(slew.next(1.1f));
//        std::cout << std::endl;
//    }
//}

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

// don't forget to clamp at -1, 1
void pan(float pan) {

    float p = (pan + 1.0f) * 0.125f;
    float left = std::cosf(2.0f * M_PI * p);
    float right = std::sinf(2.0f * M_PI * p);

    dump(pan);
    std::cout << ",";
    dump(left);
    std::cout << ",";
    dump(right);
    std::cout << std::endl;
}

void testPan() {
    for (int i = -10; i <= 10; i++) {
        pan(i/10.0f);
    }
}

void testDb() {

    int db = -120.0f;
    float amp = decibelsToAmplitude(db);

    dump(db);
    std::cout << ",";
    dump(amp);
    std::cout << std::endl;

    for (db = -60; db <= 12; db++) {
        amp = decibelsToAmplitude(db);

        dump(db);
        std::cout << ",";
        dump(amp);
        std::cout << std::endl;
    }
}

int main() {
    testDb();
}
