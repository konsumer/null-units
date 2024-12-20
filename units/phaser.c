// Phaser effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 3
#define PARAM_RATE 0     // LFO rate
#define PARAM_DEPTH 1    // Effect depth
#define PARAM_FEEDBACK 2 // Feedback amount

#define NUM_STAGES 4     // Number of all-pass stages (4 for Phase 90 style)

// State variables
static float allpassDelays[NUM_STAGES][2] = {{0.0f}}; // For stereo
static float lfoPhase[2] = {0.0f};  // LFO phase for each channel

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "phaser",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Rate parameter (0.1 to 10 Hz)
    unitInfo.params[PARAM_RATE] = (NullUnitParamInfo) {
        .name = "rate",
        .value = {.f = 1.0f},
        .min = {.f = 0.1f},
        .max = {.f = 10.0f},
        .type = NULL_PARAM_F32
    };

    // Depth parameter (0.0 to 1.0)
    unitInfo.params[PARAM_DEPTH] = (NullUnitParamInfo) {
        .name = "depth",
        .value = {.f = 0.7f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Feedback parameter (0.0 to 0.95)
    unitInfo.params[PARAM_FEEDBACK] = (NullUnitParamInfo) {
        .name = "feedback",
        .value = {.f = 0.7f},
        .min = {.f = 0.0f},
        .max = {.f = 0.95f},
        .type = NULL_PARAM_F32
    };

    return 0;
}

void destroy() {}

// All-pass filter function
float allpass(float input, float delay, int stage, int channel) {
    float output = -delay + input;
    allpassDelays[stage][channel] = input + (delay * 0.9f); // 0.9 is coefficient
    return output;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float rate = unitInfo.params[PARAM_RATE].value.f;
    float depth = unitInfo.params[PARAM_DEPTH].value.f;
    float feedback = unitInfo.params[PARAM_FEEDBACK].value.f;

    // Parameter validation
    rate = rate > 10.0f ? 10.0f : (rate < 0.1f ? 0.1f : rate);
    depth = depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth);
    feedback = feedback > 0.95f ? 0.95f : (feedback < 0.0f ? 0.0f : feedback);

    // Ensure channel index is valid
    channel = channel % 2;

    // Update LFO phase
    lfoPhase[channel] += 2.0f * M_PI * rate / sampleRate;
    if (lfoPhase[channel] >= 2.0f * M_PI) {
        lfoPhase[channel] -= 2.0f * M_PI;
    }

    // Calculate LFO value (sine wave)
    float lfoValue = sinf(lfoPhase[channel]);

    // Map LFO to all-pass filter range (minFreq to maxFreq)
    float minFreq = 200.0f;
    float maxFreq = 1800.0f;
    float lfoMapped = minFreq + (maxFreq - minFreq) * (lfoValue * 0.5f + 0.5f);

    // Calculate filter coefficient
    float theta = 2.0f * M_PI * lfoMapped / sampleRate;
    float alpha = (1.0f - tanf(theta / 2.0f)) / (1.0f + tanf(theta / 2.0f));

    // Process through phase stages
    float phased = input + feedback * allpassDelays[NUM_STAGES-1][channel];

    for (int i = 0; i < NUM_STAGES; i++) {
        phased = allpass(phased, alpha, i, channel);
    }

    // Mix dry and wet signals
    float output = input * (1.0f - depth) + phased * depth;

    // Soft clip the output
    if (output > 1.0f) output = 1.0f;
    if (output < -1.0f) output = -1.0f;

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
