#pragma once

#include <cmath>

// Adapted from github.com/MarcBoule/MindMeldModular/src/comp/VuMeters.hpp
// GPL3 licensed: github.com/MarcBoule/MindMeldModular/LICENSE.md
struct Track {

    static constexpr float lambda = 30.f; // Inverse time constant in 1/seconds

    float leftRms = 0.0f;
    float rightRms = 0.0f;

    float leftPeak = 0.0f;
    float rightPeak = 0.0f;

    void process(float left, float right, float deltaTime) {

        //---------------------------------------------
        // RMS

        leftRms += (std::pow(left, 2) - leftRms) * lambda * deltaTime;

        rightRms += (std::pow(right, 2) - rightRms) * lambda * deltaTime;

        //---------------------------------------------
        // Peak

        float leftAbs = std::fabs(left);
        if (leftAbs >= leftPeak) {
            leftPeak = leftAbs;
        } else {
            leftPeak += (leftAbs - leftPeak) * lambda * deltaTime;
        }

        float rightAbs = std::fabs(right);
        if (rightAbs >= rightPeak) {
            rightPeak = rightAbs;
        } else {
            rightPeak += (rightAbs - rightPeak) * lambda * deltaTime;
        }
    }
};
