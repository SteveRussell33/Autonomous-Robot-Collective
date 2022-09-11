#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

//include "../../lib/bogaudio/dsp/signal.hpp"

void dump(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(3);
    std::cout << v;
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

static constexpr float kTwoPi = 2.0f * M_PI;

// This wavefolding algorithm is derived from a Max/MSP patch that was
// created by Randy Jones of Madrona Labs.
inline float wavefold(float in, float timbre) {

    float ampOffset = timbre * 2.0f + 0.1f;
    float phaseOffset = timbre + 0.25f;

    float phase = in * ampOffset;
    phase = phase + phaseOffset;

    // TODO should we use a fast approximation for cosf()?
    return std::cosf(kTwoPi * (phase + 0.75));
}

void fold() {

    std::cout << "T,A,B" << std::endl;

    float timbre = 0.4f;
    float amplitude = 0.1f;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {

            float t = i + j/100.0f;

            float a = std::sinf(kTwoPi * t);
            a = a * amplitude;

            float b = wavefold(a, timbre);
            b = b * amplitude;

            dump(t);
            std::cout << ",";
            dump(a);
            std::cout << ",";
            dump(b);
            std::cout << std::endl;
        }
        
        //amplitude = amplitude - 1.0f/20.0f;
    }
}

int main() {
    fold();
}
