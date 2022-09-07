#pragma once

#include <cmath>

// This class is adapted from github.com/MarcBoule/MindMeldModular/src/comp/VuMeters.hpp.
// License: github.com/MarcBoule/MindMeldModular/LICENSE.md
struct Meter {

    static constexpr float lambda = 30.f; // Inverse time constant in 1/seconds

    float rms = 0.0f;
    float peak = 0.0f;

    void process(float in, float deltaTime) {

        rms += (in*in - rms) * lambda * deltaTime;

        float absv = std::fabs(in);
        if (absv >= peak) {
            peak = absv;
        } else {
            peak += (absv - peak) * lambda * deltaTime;
        }
    }
};
