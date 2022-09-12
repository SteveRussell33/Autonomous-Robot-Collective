#pragma once

#include <cmath>

#include "earlevel/Biquad.h"

namespace rm {
namespace dsp {

//--------------------------------------------------------------
// signal
//--------------------------------------------------------------

// https://www.kvraudio.com/forum/viewtopic.php?p=2779936&sid=e1e44d1b6ec3fd37f76e4a8ca1ed422a#p2779936
static inline float softClip(float x) {

    // Hard Clip: clamp(x, -1, 1)
    x = 0.5f * (std::fabs(x + 1.0f) - std::fabs(x - 1.0f));

    // Soft Clip: Simple f(x) = 1.5x - 0.5x^3 waveshaper
    x =  1.5f * x - 0.5f * x * x * x;

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
    const double Q[kFilters] = {
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

    // Nuke everything above the Nyquist frequency.
    TwelvePoleLpf upLpf;
    TwelvePoleLpf downLpf;

  public:

    Oversample(int oversample_) : oversample(oversample_) {
    }

    void sampleRateChange(float sampleRate) {

        float nyquist = sampleRate / 2.0f;
        float oversampleRate = sampleRate * oversample;

        upLpf.setCutoff(nyquist, oversampleRate);
        downLpf.setCutoff(nyquist, oversampleRate);
    }

    void upsample(float in, float* buffer) {

        buffer[0] = upLpf.process(in);

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
} // namespace rm
