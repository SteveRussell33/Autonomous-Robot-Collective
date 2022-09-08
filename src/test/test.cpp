#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "../dsp/dsp.hpp"

int main() {

    std::cout << "-----------------------------------" << std::endl;

    for (float amp = 0.0; amp < 1.3f; amp += 0.02) {
        float db = ampToDb(amp);
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << amp << ": " << db << std::endl;
    }

    std::cout << "-----------------------------------" << std::endl;
}