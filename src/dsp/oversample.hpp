#pragma once

#include "earlevel/Biquad.h"

const int kMaxOversample = 16;

struct TwelvePoleLpf {

    static const int kFilters = 6;
    Biquad filter[kFilters];

    void setCutoff(float cutoff, float sampleRate) {

        double Fc = cutoff / sampleRate;

        // https://www.earlevel.com/main/2016/09/29/cascading-filters/
        double q[kFilters] = {0.50431448, 0.54119610, 0.63023621, 0.82133982, 1.3065630, 3.8306488};

        for (int i = 0; i < kFilters; i++) {
            filter[i].setBiquad(bq_type_lowpass, Fc, q[i], 0);
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

struct Oversample {

    int oversample;

    // Nuke everything above the Nyquist frequency.
    TwelvePoleLpf upLpf;
    TwelvePoleLpf downLpf;

    Oversample(int oversample_) : oversample(oversample_) {}

    void sampleRateChange(float sampleRate) {

        float nyquist = sampleRate / 2.0f;
        float oversampleRate = sampleRate * oversample;

        upLpf.setCutoff(nyquist, oversampleRate);
        downLpf.setCutoff(nyquist, oversampleRate);
    }

    void upsample(float in, float* buffer) {

        // Apply gain to compensate for zero-interpolation
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
