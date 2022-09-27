#pragma once

#include <cmath>

#include "earlevel/Biquad.h"
#include "rack.hpp"

using namespace rack;

namespace arc {
namespace dsp {

// TODO lookup tables

inline float frequencyToCV(float frequency) {
    return log2f(frequency / rack::dsp::FREQ_C4);
}

inline float cvToFrequency(float cv) {
    return powf(2.0, cv) * rack::dsp::FREQ_C4;
}

template <typename T> T amplitudeToDecibels(T amp) {
    return simd::log10(amp) * 20;
}

template <typename T> T decibelsToAmplitude(T db) {
    if (db <= -60.0f) {
        return 0.0f;
    }
    return std::pow(10, db / 20);
}

//--------------------------------------------------------------
// Pan
//--------------------------------------------------------------

struct Pan {

    float left = 0.7071068f;
    float right = 0.7071068f;

    void next(float pan) {

        pan = clamp(pan, -1.0f, 1.0f);
        pan = (pan + 1.0f) * 0.125f;

        // TODO use lookup tables
        left = std::cosf(2.0f * M_PI * pan);
        right = std::sinf(2.0f * M_PI * pan);
    }
};

//--------------------------------------------------------------
// LinearRamp
//--------------------------------------------------------------

class LinearRamp {

    float sampleRate = 1.0f;
    float time = 1.0f; // in seconds
    float divisor = 1.0f;

    float target = 0.0f;
    float increment = 0.0f;
    float value = 0.0f;

    void recalc() {
        divisor = 1.0f / (sampleRate * time);
    }

  public:

    void onSampleRateChange(float sampleRate_) {
        assert(sampleRate_ > 0.0f);
        sampleRate = sampleRate_;
        recalc();
    }

    void setTime(float time_) {
        assert(time_ > 0.0f);
        time = time_;
        recalc();
    }

    void setValue(float value_) {
        value = value_;
    }

    float next(float target_) {

        // done already
        if (target_ == value) {
            return value;
        }

        // new target
        if (target != target_) {
            target = target_;
            increment = (target - value) * divisor;
        }

        // increment the value
        bool rising = (target > value);
        value += increment;

        // rising
        if (rising) {
            if (value > target) {
                value = target;
            }
        }
        // falling
        else {
            if (value < target) {
                value = target;
            }
        }

        return value;
    }
};

//--------------------------------------------------------------
// soft clip
//--------------------------------------------------------------

// https://www.kvraudio.com/forum/viewtopic.php?p=2779936&sid=e1e44d1b6ec3fd37f76e4a8ca1ed422a#p2779936
static inline float softClip(float x) {

    // Hard Clip: clamp(x, -1, 1)
    x = 0.5f * (std::fabs(x + 1.0f) - std::fabs(x - 1.0f));

    // Soft Clip: Simple f(x) = 1.5x - 0.5x^3 waveshaper
    x = 1.5f * x - 0.5f * x * x * x;

    // Remove the introduced gain
    return x * 0.6666667;
}

//--------------------------------------------------------------
// TwelvePoleLpf
//--------------------------------------------------------------

struct TwelvePoleLpf {

  private:

    static const int kFilters = 6;

    // https://www.earlevel.com/main/2016/09/29/cascading-filters/
    static constexpr double Q[kFilters] = {
        0.50431448, 0.54119610, 0.63023621, 0.82133982, 1.3065630, 3.8306488};

    Biquad filter[kFilters];

  public:

    void setCutoff(float cutoff, float sampleRate) {

        double Fc = cutoff / sampleRate;

        for (int i = 0; i < kFilters; i++) {
            filter[i].setBiquad(bq_type_lowpass, Fc, Q[i], 0);
        }
    }

    float process(float in) {
        float out = in;
        for (int i = 0; i < kFilters; i++) {
            out = filter[i].process(out);
        }
        return out;
    }
};

//--------------------------------------------------------------
// Oversample
//--------------------------------------------------------------

const int kMaxOversample = 16;

struct Oversample {

  private:

    int oversample;

    TwelvePoleLpf upLpf;
    TwelvePoleLpf downLpf;

  public:

    Oversample(int oversample_) : oversample(oversample_) {
    }

    void onSampleRateChange(float sampleRate) {

        float nyquist = sampleRate / 2.0f;
        float oversampleRate = sampleRate * oversample;

        upLpf.setCutoff(nyquist, oversampleRate);
        downLpf.setCutoff(nyquist, oversampleRate);
    }

    void upsample(float in, float* buffer) {

        // Apply gain to compensate for filtering
        buffer[0] = upLpf.process(in * oversample);

        // Interpolate with zeros
        for (int i = 1; i < oversample; ++i) {
            buffer[i] = upLpf.process(0.0f);
        }
    }

    float downsample(float* buffer) {
        for (int i = 0; i < oversample; ++i) {
            downLpf.process(buffer[i]);
        }
        return buffer[0];
    }
};

} // namespace dsp
} // namespace arc
