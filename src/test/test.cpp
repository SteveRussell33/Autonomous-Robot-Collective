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

void csv() {

    std::cout << "T,A,B" << std::endl;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {

            float t = i + j/100.0f;

            float a = foo();
            a = a * amplitude;

            float b = bar();
            b = b * amplitude;

            dump(t);
            std::cout << ",";
            dump(a);
            std::cout << ",";
            dump(b);
            std::cout << std::endl;
        }
    }
}

int main() {
    csv();
}
