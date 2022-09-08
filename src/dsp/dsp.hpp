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
