// HeadTracker v2.40 Expo: curve after gain/inversion, before channel routing.
#pragma once
#include <stdint.h>
#include <math.h>

static inline uint16_t expoChannel(float deltaUs, uint16_t centerUs,
                                   uint16_t minUs, uint16_t maxUs,
                                   uint8_t expoPercent)
{
    // Invalid/equal travel collapses to the configured minimum.
    if (minUs >= maxUs) return minUs;
    const float lo = minUs, hi = maxUs;
    const float center = fmaxf(lo, fminf(hi, centerUs));
    if (!isfinite(deltaUs)) return static_cast<uint16_t>(center);
    float output;
    if (expoPercent == 0) {
        // Match v2.40 truncation exactly for valid, representable inputs.
        // Clamp BEFORE conversion, avoiding v2.40's negative float -> uint16 UB.
        output = center + deltaUs;
    } else {
        const float span = deltaUs < 0.0f ? center - lo : hi - center;
        if (span <= 0.0f) return static_cast<uint16_t>(center);
        const float x = fmaxf(-1.0f, fminf(1.0f, deltaUs / span));
        const float e = (expoPercent > 100 ? 100 : expoPercent) / 100.0f;
        output = center + span * x * ((1.0f - e) + e * x * x);
    }
    return static_cast<uint16_t>(fmaxf(lo, fminf(hi, output)));
}
