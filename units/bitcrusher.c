// Bitcrusher effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 4
#define PARAM_BITS 0      // Bit depth reduction
#define PARAM_RATE 1      // Sample rate reduction
#define PARAM_NOISE 2     // Digital noise amount
#define PARAM_MIX 3       // Wet/dry mix

// State variables
static float sampleHold[2] = {0.0f, 0.0f};  // Sample & hold state
static float phaseAcc[2] = {0.0f, 0.0f};    // Phase accumulator for rate reduction
static uint32_t noiseState[2] = {12345, 67890}; // Simple PRNG state

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "bitcrusher",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Bits parameter (1.0 to 16.0)
    unitInfo.params[PARAM_BITS] = (NullUnitParamInfo) {
        .name = "bits",
        .value = {.f = 16.0f},
        .min = {.f = 1.0f},
        .max = {.f = 16.0f},
        .type = NULL_PARAM_F32
    };

    // Rate parameter (0.0 to 1.0, where 1.0 is full sample rate)
    unitInfo.params[PARAM_RATE] = (NullUnitParamInfo) {
        .name = "rate",
        .value = {.f = 1.0f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Noise parameter (0.0 to 1.0)
    unitInfo.params[PARAM_NOISE] = (NullUnitParamInfo) {
        .name = "noise",
        .value = {.f = 0.0f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Mix parameter (0.0 to 1.0)
    unitInfo.params[PARAM_MIX] = (NullUnitParamInfo) {
        .name = "mix",
        .value = {.f = 1.0f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    return 0;
}

void destroy() {}

// Simple PRNG for digital noise
uint32_t xorshift32(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

// Generate noise sample
float generateNoise(uint32_t* state) {
    return (float)(xorshift32(state)) / (float)0xFFFFFFFF * 2.0f - 1.0f;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float bits = unitInfo.params[PARAM_BITS].value.f;
    float rate = unitInfo.params[PARAM_RATE].value.f;
    float noise = unitInfo.params[PARAM_NOISE].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Parameter validation
    bits = bits > 16.0f ? 16.0f : (bits < 1.0f ? 1.0f : bits);
    rate = rate > 1.0f ? 1.0f : (rate < 0.0f ? 0.0f : rate);
    noise = noise > 1.0f ? 1.0f : (noise < 0.0f ? 0.0f : noise);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    float crushedSignal = input;

    // Sample rate reduction
    if (rate < 1.0f) {
        // Calculate effective sample rate
        float downSample = 1.0f + (1.0f - rate) * 100.0f; // 1x to 100x reduction

        // Update phase accumulator
        phaseAcc[channel] += 1.0f;

        // Check if we should update the sample & hold
        if (phaseAcc[channel] >= downSample) {
            sampleHold[channel] = input;
            phaseAcc[channel] -= downSample;
        }

        crushedSignal = sampleHold[channel];
    }

    // Bit depth reduction
    if (bits < 16.0f) {
        float steps = powf(2.0f, bits);
        crushedSignal = floorf(crushedSignal * steps + 0.5f) / steps;
    }

    // Add digital noise
    if (noise > 0.0f) {
        float noiseSignal = generateNoise(&noiseState[channel]);
        // Scale noise based on signal level for more realistic effect
        float noiseAmount = noise * fabsf(crushedSignal) * 0.5f;
        crushedSignal += noiseSignal * noiseAmount;
    }

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (crushedSignal * mix);

    // Ensure output is valid
    if (isnan(output)) output = 0.0f;
    if (isinf(output)) output = 0.0f;

    return output;
}

NullUnitnInfo* get_info() {
    return &unitInfo;
}

void param_set(uint8_t paramId, NullUnitParamValue* value) {
    if (paramId >= PARAM_COUNT) {
        return;
    }
    unitInfo.params[paramId].value = *value;
}

NullUnitParamValue* param_get(uint8_t paramId) {
    if (paramId >= PARAM_COUNT) {
        return NULL;
    }
    return &unitInfo.params[paramId].value;
}
