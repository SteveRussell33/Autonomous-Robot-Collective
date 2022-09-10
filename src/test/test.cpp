#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "../../lib/bogaudio/dsp/signal.hpp"

void dumpFloat(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    std::cout << v << std::endl;
}

int main() {

    const float sampleRate = 48000.0f;
    const float slewTime = 5.0f; // millis

	bogaudio::dsp::SlewLimiter slew;
    slew.setParams(sampleRate, slewTime, 1.0f);

    std::cout << "-----------------------------------" << std::endl;

    slew.setLast(0.9); 
    for (int i = 0; i < 100; i++) {
        dumpFloat(slew.next(1.1f));
    }
    
    std::cout << "-----------------------------------" << std::endl;
}
