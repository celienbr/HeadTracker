// Deadzone (microseconds of input command) per side, before Expo.
// A nonzero deadzone holds center and rescales the remaining command range,
// so the previously configured end-stops stay reachable.
#pragma once
#include "expo_curve.h"
#include <math.h>
#include <stdint.h>

static inline uint16_t expoDeadzoneChannel(float deltaUs, uint16_t centerUs,
                                           uint16_t minUs, uint16_t maxUs,
                                           uint8_t expoPercent,
                                           uint16_t deadLowUs, uint16_t deadHighUs)
{
    if (!isfinite(deltaUs) || minUs >= maxUs)
        return expoChannel(deltaUs, centerUs, minUs, maxUs, expoPercent);
    if (deadLowUs == 0 && deadHighUs == 0)
        return expoChannel(deltaUs, centerUs, minUs, maxUs, expoPercent);
    const float center = fmaxf(float(minUs), fminf(float(maxUs), float(centerUs)));
    const bool negative = deltaUs < 0.0f;
    const float span = negative ? center - minUs : maxUs - center;
    if (span < 1.0f) return static_cast<uint16_t>(center);
    const float requested = negative ? deadLowUs : deadHighUs;
    const float dead = fminf(requested, span - 1.0f);
    const float magnitude = fabsf(deltaUs);
    if (magnitude <= dead) return static_cast<uint16_t>(center);
    if (dead == 0.0f)
        return expoChannel(deltaUs, centerUs, minUs, maxUs, expoPercent);
    const float linearized = fminf(1.0f, (magnitude - dead) / (span - dead)) * span;
    return expoChannel(negative ? -linearized : linearized,
                       centerUs, minUs, maxUs, expoPercent);
}
