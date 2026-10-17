#pragma once
#include "arduinojsonwrp.h"
#include <string.h>

// nRF52840 flash writes are 4-byte aligned. Include the JSON terminator.
// A zero result means no write should be attempted.
static inline size_t serializeFlashSettings(JsonDocument &json, uint8_t *buffer, size_t capacity)
{
    const size_t size = measureJson(json);
    if (size + 1 > capacity) return 0;
    const size_t padded = (size + 1 + 3) & ~size_t(3);
    if (padded > capacity) return 0;
    memset(buffer, 0, padded);
    return serializeJson(json, buffer, capacity) == size ? padded : 0;
}
