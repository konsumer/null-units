// Compressor effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 7
#define PARAM_THRESHOLD 0  // Threshold level
#define PARAM_RATIO 1     // Compression ratio
#define PARAM_ATTACK 2    // Attack time
#define PARAM_RELEASE 3   // Release time
#define PARAM_KNEE 4      // Soft knee width
#define PARAM_MAKEUP 5    // Makeup gain
#define PARAM_MIX 6       // Parallel compression mix

// State variables per channel
typedef struct {
    float envelope;       // Envelope follower
    float gainReduction; // Current gain reduction
    float peakEnv;       // Peak envelope for LED meter simulation
} CompressorState;

static CompressorState state[2];  // For stereo

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "compressor",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Threshold parameter (-60.0 to 0.0 dB)
    unitInfo.params[PARAM_THRESHOLD] = (NullUnitParamInfo) {
        .name = "threshold",
        .value = {.f = -20.0f},
        .min = {.f = -60.0f},
        .max = {.f = 0.0f},
        .type = NULL_PARAM_F32
    };

    // Ratio parameter (1.0 to 20.0)
    unitInfo.params[PARAM_RATIO] = (NullUnitParamInfo) {
        .name = "ratio",
        .value = {.f = 4.0f},
        .min = {.f = 1.0f},
        .max = {.f = 20.0f},
        .type = NULL_PARAM_F32
    };

    // Attack parameter (0.1 to 100.0 ms)
    unitInfo.params[PARAM_ATTACK] = (NullUnitParamInfo) {
        .name = "attack",
        .value = {.f = 10.0f},
        .min = {.f = 0.1f},
        .max = {.f = 100.0f},
        .type = NULL_PARAM_F32
    };

    // Release parameter (10.0 to 1000.0 ms)
    unitInfo.params[PARAM_RELEASE] = (NullUnitParamInfo) {
        .name = "release",
        .value = {.f = 100.0f},
        .min = {.f = 10.0f},
        .max = {.f = 1000.0f},
        .type = NULL_PARAM_F32
    };

    // Knee parameter (0.0 to 24.0 dB)
    unitInfo.params[PARAM_KNEE] = (NullUnitParamInfo) {
        .name = "knee",
        .value = {.f = 6.0f},
        .min = {.f = 0.0f},
        .max = {.f = 24.0f},
        .type = NULL_PARAM_F32
    };

    // Makeup gain parameter (0.0 to 24.0 dB)
    unitInfo.params[PARAM_MAKEUP] = (NullUnitParamInfo) {
        .name = "makeup",
        .value = {.f = 0.0f},
        .min = {.f = 0.0f},
        .max = {.f = 24.0f},
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

    // Initialize state
    for (int ch = 0; ch < 2; ch++) {
        state[ch].envelope = 0.0f;
        state[ch].gainReduction = 1.0f;
        state[ch].peakEnv = 0.0f;
    }

    return 0;
}

void destroy() {}

// Convert dB to linear
float dB2Linear(float dB) {
    return powf(10.0f, dB / 20.0f);
}

// Convert linear to dB
float linear2dB(float linear) {
    return 20.0f * log10f(fmaxf(linear, 1e-6f));
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float threshold = unitInfo.params[PARAM_THRESHOLD].value.f;
    float ratio = unitInfo.params[PARAM_RATIO].value.f;
    float attack = unitInfo.params[PARAM_ATTACK].value.f;
    float release = unitInfo.params[PARAM_RELEASE].value.f;
    float knee = unitInfo.params[PARAM_KNEE].value.f;
    float makeup = unitInfo.params[PARAM_MAKEUP].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Parameter validation
    threshold = threshold > 0.0f ? 0.0f : (threshold < -60.0f ? -60.0f : threshold);
    ratio = ratio > 20.0f ? 20.0f : (ratio < 1.0f ? 1.0f : ratio);
    attack = attack > 100.0f ? 100.0f : (attack < 0.1f ? 0.1f : attack);
    release = release > 1000.0f ? 1000.0f : (release < 10.0f ? 10.0f : release);
    knee = knee > 24.0f ? 24.0f : (knee < 0.0f ? 0.0f : knee);
    makeup = makeup > 24.0f ? 24.0f : (makeup < 0.0f ? 0.0f : makeup);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    // Calculate envelope
    float inputAbs = fabsf(input);
    float inputdB = linear2dB(inputAbs);

    // Calculate attack and release times
    float attackTime = expf(-1.0f / (sampleRate * attack * 0.001f));
    float releaseTime = expf(-1.0f / (sampleRate * release * 0.001f));

    // Envelope follower
    float envelope = state[channel].envelope;
    if (inputAbs > envelope) {
        envelope = attackTime * envelope + (1.0f - attackTime) * inputAbs;
    } else {
        envelope = releaseTime * envelope + (1.0f - releaseTime) * inputAbs;
    }
    state[channel].envelope = envelope;

    // Convert envelope to dB
    float envelopedB = linear2dB(envelope);

    // Compute gain reduction with soft knee
    float gainReduction = 0.0f;

    if (2.0f * (envelopedB - threshold) < -knee) {
        // Below knee
        gainReduction = 0.0f;
    } else if (2.0f * (envelopedB - threshold) > knee) {
        // Above knee
        gainReduction = ((ratio - 1.0f) / ratio) * (envelopedB - threshold);
    } else {
        // In knee
        float kneeRange = envelopedB - threshold + knee/2.0f;
        gainReduction = ((ratio - 1.0f) / ratio) *
                       (kneeRange * kneeRange) / (2.0f * knee);
    }

    // Smooth gain reduction changes
    float targetReduction = dB2Linear(-gainReduction);
    state[channel].gainReduction = targetReduction;

    // Apply gain reduction and makeup gain
    float compressedSignal = input * state[channel].gainReduction * dB2Linear(makeup);

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (compressedSignal * mix);

    // Update peak envelope for metering
    state[channel].peakEnv = fmaxf(gainReduction,
                                  state[channel].peakEnv * expf(-1.0f / (sampleRate * 0.1f)));

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
