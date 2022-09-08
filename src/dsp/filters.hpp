#pragma once

#include "earlevel/Biquad.h"

#include "dsp.hpp"

struct TwelvePoleLpf {

    static const int kFilters = 6;
    Biquad filter[kFilters];

    void setCutoff(float cutoff, float sampleRate) {

        double Fc = cutoff / sampleRate;

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

///////////////////////////////////////////////////////////////////////
//
// Q values for each filter to achieve Butterworth response
// for lowpass and highpass filters
//
// From https://www.earlevel.com/main/2016/09/29/cascading-filters/
//
// 2-pole:
//
//     0.70710678
//
// 4-pole:
//
//     0.54119610
//     1.3065630
//
// 6-pole:
//
//     0.51763809
//     0.70710678
//     1.9318517
//
// 8-pole:
//
//     0.50979558
//     0.60134489
//     0.89997622
//     2.5629154
//
// 10-pole:
//
//     0.50623256
//     0.56116312
//     0.70710678
//     1.1013446
//     3.1962266
//
// 12-pole:
//
//     0.50431448
//     0.54119610
//     0.63023621
//     0.82133982
//     1.3065630
//     3.8306488
//
// 14-pole:
//
//     0.50316379
//     0.52972649
//     0.59051105
//     0.70710678
//     0.93979296
//     1.5138713
//     4.4657021
//
// 16-pole:
//
//     0.50241929
//     0.52249861
//     0.56694403
//     0.64682178
//     0.78815462
//     1.0606777
//     1.7224471
//     5.1011486
//
