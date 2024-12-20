// Ring Modulator effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 5
#define PARAM_FREQ 0      // Carrier frequency
#define PARAM_WAVE 1      // Carrier waveform
#define PARAM_DEPTH 2     // Modulation depth
#define PARAM_OFFSET 3    // DC offset for asymmetric modulation
#define PARAM_MIX 4       // Wet/dry mix

// Carrier waveform types
#define WAVE_SINE 0
#define WAVE_SQUARE 1
#define WAVE_TRI 2
#define WAVE_SAW 3

static float phase[2] = {0.0f, 0.0f};  // Oscillator phase per channel

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "ringmod",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Frequency parameter (10.0 to 2000.0 Hz)
    unitInfo.params[PARAM_FREQ] = (NullUnitParamInfo) {
        .name = "freq",
        .value = {.f = 440.0f},
        .min = {.f = 10.0f},
        .max = {.f = 2000.0f},
        .type = NULL_PARAM_F32
    };

    // Waveform parameter (0 to 3)
    unitInfo.params[PARAM_WAVE] = (NullUnitParamInfo) {
        .name = "wave",
        .value = {.i = WAVE_SINE},
        .min = {.i = 0},
        .max = {.i = 3},
        .type = NULL_PARAM_I32
    };

    // Depth parameter (0.0 to 1.0)
    unitInfo.params[PARAM_DEPTH] = (NullUnitParamInfo) {
        .name = "depth",
        .value = {.f = 1.0f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Offset parameter (-1.0 to 1.0)
    unitInfo.params[PARAM_OFFSET] = (NullUnitParamInfo) {
        .name = "offset",
        .value = {.f = 0.0f},
        .min = {.f = -1.0f},
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

// Waveform generators
float squareWave(float phase) {
    return phase < M_PI ? 1.0f : -1.0f;
}

float triangleWave(float phase) {
    float normPhase = phase / (2.0f * M_PI);
    return 2.0f * fabsf(2.0f * (normPhase - floorf(normPhase + 0.5f))) - 1.0f;
}

float sawWave(float phase) {
    float normPhase = phase / (2.0f * M_PI);
    return 2.0f * (normPhase - floorf(normPhase + 0.5f));
}

// Get carrier signal based on waveform type
float getCarrier(float phase, int waveform) {
    switch(waveform) {
        case WAVE_SQUARE:
            return squareWave(phase);
        case WAVE_TRI:
            return triangleWave(phase);
        case WAVE_SAW:
            return sawWave(phase);
        case WAVE_SINE:
        default:
            return sinf(phase);
    }
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float freq = unitInfo.params[PARAM_FREQ].value.f;
    int wave = unitInfo.params[PARAM_WAVE].value.i;
    float depth = unitInfo.params[PARAM_DEPTH].value.f;
    float offset = unitInfo.params[PARAM_OFFSET].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Parameter validation
    freq = freq > 2000.0f ? 2000.0f : (freq < 10.0f ? 10.0f : freq);
    wave = wave > 3 ? 3 : (wave < 0 ? 0 : wave);
    depth = depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth);
    offset = offset > 1.0f ? 1.0f : (offset < -1.0f ? -1.0f : offset);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    // Update oscillator phase
    phase[channel] += 2.0f * M_PI * freq / sampleRate;
    if (phase[channel] >= 2.0f * M_PI) {
        phase[channel] -= 2.0f * M_PI;
    }

    // Get carrier signal
    float carrier = getCarrier(phase[channel], wave);

    // Apply depth and offset
    carrier = (carrier * depth) + offset;

    // Apply ring modulation
    float modulated = input * carrier;

    // Scale output to compensate for modulation gain
    modulated *= 0.5f;

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (modulated * mix);

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
