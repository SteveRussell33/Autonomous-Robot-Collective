#pragma once

#include <cmath>
#include <math.h>

const float kTwoPi = 2.0f * M_PI;

// A fast approximation for tanh(), by Aleksey Vaneev.
// https://www.kvraudio.com/forum/viewtopic.php?p=5447225#p5447225
inline float fastTanh(const float x) {

    const float ax = fabs(x);
    const float x2 = x * x;
    const float z =
        x * (0.773062670268356 + ax + (0.757118539838817 + 0.0139332362248817 * x2 * x2) * x2 * ax);

    return (z / (0.795956503022967 + fabs(z)));
}

// A wavefolding algorithm, derived from a Max/MSP patch that was 
// created by Randy Jones of Madrona Labs.
inline float wavefold(float in, float timbre) {

    float ampOffset = timbre * 2.0f + 0.1f;
    float phaseOffset = timbre + 0.25f;

    float amp = in * ampOffset;

    // TODO should we use a fast approximation for cosf()?
    return std::cosf(kTwoPi * (amp + phaseOffset));
}

