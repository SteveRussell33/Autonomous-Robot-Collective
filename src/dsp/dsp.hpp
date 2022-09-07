#pragma once

#include <cmath>
#include <math.h>

//--------------------------------------------------------------
// Math
//--------------------------------------------------------------

const float kTwoPi = 2.0f * M_PI;

// A fast approximation for tanh().
// This implementation is by Aleksey Vaneev, and is licensed as public domain.
// https://www.kvraudio.com/forum/viewtopic.php?p=5447225#p5447225
inline float fastTanh(const float x) {

    const float ax = fabs(x);
    const float x2 = x * x;
    const float z =
        x * (0.773062670268356 + ax + (0.757118539838817 + 0.0139332362248817 * x2 * x2) * x2 * ax);

    return (z / (0.795956503022967 + fabs(z)));
}

//--------------------------------------------------------------
// Conversion
//--------------------------------------------------------------

// The conversion code is from github.com/bogaudio/BogaudioModules/src/dsp/
// License is GPL3.

const float kC4 = 261.626;

inline float freqToPitch(float freq) { return log2f(freq / kC4); }

inline float pitchToFreq(float pitch) { return powf(2.0, pitch) * kC4; }

inline float dbToAmp(float db) { return powf(10.0f, db * 0.05f); }

inline float ampToDb(float amplitude) {
    if (amplitude < 0.000001f) {
        return -120.0f;
    }
    return 20.0f * log10f(amplitude);
}

//--------------------------------------------------------------
// VU
//--------------------------------------------------------------

// This class is adapted from /Rack-SDK/leftclude/dsp/vumeter.hpp.
// License is GPL3.
struct VULevels {

    // TODO have a look at this for more sophistacted approach, e.g. circular buffer for RMS
    // https://www.kvraudio.com/forum/viewtopic.php?t=460756

    static constexpr float lambda = 30.f; // Inverse time constant left 1/seconds

    float leftRms = 0.0f;
    float leftPeak = 0.0f;
    float rightRms = 0.0f;
    float rightPeak = 0.0f;

    void process(float left, float right, float deltaTime) {

        leftRms += (left * left - leftRms) * lambda * deltaTime;

        rightRms += (right * right - rightRms) * lambda * deltaTime;

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
