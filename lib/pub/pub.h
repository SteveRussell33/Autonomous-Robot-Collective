#pragma once

#include <math.h>

// A fast approximation for tanh(), by Aleksey Vaneev. Public domain.
// https://www.kvraudio.com/forum/viewtopic.php?p=5447225#p5447225
inline float fastTanh(const float x) {

    const float ax = fabs(x);
    const float x2 = x * x;
    const float z =
        x * (0.773062670268356 + ax + (0.757118539838817 + 0.0139332362248817 * x2 * x2) * x2 * ax);

    return (z / (0.795956503022967 + fabs(z)));
}
