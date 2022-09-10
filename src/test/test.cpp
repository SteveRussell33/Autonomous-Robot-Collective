#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "../../lib/bogaudio/dsp/signal.hpp"

float dist(float x, float low, float high) {
    return (x - low) / (high - low);
}

void dumpFloat(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    std::cout << v << std::endl;
}

int main() {

    const float sampleRate = 48000.0f;
    const float kMinDb = -72.0f;
    const float kMaxDb = 6.0f;

	bogaudio::dsp::SlewLimiter dbSlew;
    dbSlew.setParams(sampleRate, 5.0f, kMaxDb - kMinDb);

    std::cout << "-----------------------------------" << std::endl;

    for (int i = 0; i < 100; i++) {
        //dumpFloat(dbSlew.next(kMinDb));
        dumpFloat(dbSlew.next(6.0f));
    }
    
    std::cout << "-----------------------------------" << std::endl;
}
