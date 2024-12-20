// Auto-wah (envelope follower) effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 6
#define PARAM_SENSITIVITY 0  // Envelope sensitivity
#define PARAM_ATTACK 1      // Envelope attack time
#define PARAM_RELEASE 2     // Envelope release time
#define PARAM_RESONANCE 3   // Filter resonance
#define PARAM_RANGE 4       // Frequency range
#define PARAM_UP 5          // Direction (up/down)

// State variables per channel
typedef struct {
    float envelope;         // Envelope follower
    float filterState1;     // Filter states
    float filterState2;
} WahState;

static WahState state[2];   // For stereo

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "autowah",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Sensitivity parameter (0.0 to 1.0)
    unitInfo.params[PARAM_SENSITIVITY] = (NullUnitParamInfo) {
        .name = "sensitivity",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Attack parameter (1.0 to 200.0 ms)
    unitInfo.params[PARAM_ATTACK] = (NullUnitParamInfo) {
        .name = "attack",
        .value = {.f = 20.0f},
        .min = {.f = 1.0f},
        .max = {.f = 200.0f},
        .type = NULL_PARAM_F32
    };

    // Release parameter (1.0 to 500.0 ms)
    unitInfo.params[PARAM_RELEASE] = (NullUnitParamInfo) {
        .name = "release",
        .value = {.f = 100.0f},
        .min = {.f = 1.0f},
        .max = {.f = 500.0f},
        .type = NULL_PARAM_F32
    };

    // Resonance parameter (0.0 to 0.99)
    unitInfo.params[PARAM_RESONANCE] = (NullUnitParamInfo) {
        .name = "resonance",
        .value = {.f = 0.7f},
        .min = {.f = 0.0f},
        .max = {.f = 0.99f},
        .type = NULL_PARAM_F32
    };

    // Range parameter (0.0 to 1.0)
    unitInfo.params[PARAM_RANGE] = (NullUnitParamInfo) {
        .name = "range",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Direction parameter (up/down)
    unitInfo.params[PARAM_UP] = (NullUnitParamInfo) {
        .name = "up",
        .value = {.i = true},
        .min = {.i = false},
        .max = {.i = true},
        .type = NULL_PARAM_BOOL
    };

    // Initialize state
    for (int ch = 0; ch < 2; ch++) {
        state[ch].envelope = 0.0f;
        state[ch].filterState1 = 0.0f;
        state[ch].filterState2 = 0.0f;
    }

    return 0;
}

void destroy() {}

// State variable filter implementation
float processSVF(float input, float cutoff, float resonance, float* state1, float* state2, float sampleRate) {
    float f = 2.0f * sinf(M_PI * cutoff / sampleRate);
    float q = resonance;
    float fb = q + q/(1.0f - f);

    // Process filter
    float high = input - (*state1) * fb - *state2;
    float band = f * high + *state1;
    float low = f * band + *state2;

    // Update states
    *state1 = band;
    *state2 = low;

    // Return bandpass output for wah effect
    return band;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float sensitivity = unitInfo.params[PARAM_SENSITIVITY].value.f;
    float attack = unitInfo.params[PARAM_ATTACK].value.f;
    float release = unitInfo.params[PARAM_RELEASE].value.f;
    float resonance = unitInfo.params[PARAM_RESONANCE].value.f;
    float range = unitInfo.params[PARAM_RANGE].value.f;
    bool up = unitInfo.params[PARAM_UP].value.i;

    // Parameter validation
    sensitivity = sensitivity > 1.0f ? 1.0f : (sensitivity < 0.0f ? 0.0f : sensitivity);
    attack = attack > 200.0f ? 200.0f : (attack < 1.0f ? 1.0f : attack);
    release = release > 500.0f ? 500.0f : (release < 1.0f ? 1.0f : release);
    resonance = resonance > 0.99f ? 0.99f : (resonance < 0.0f ? 0.0f : resonance);
    range = range > 1.0f ? 1.0f : (range < 0.0f ? 0.0f : range);

    // Ensure channel index is valid
    channel = channel % 2;

    // Calculate envelope
    float inputAbs = fabsf(input);
    float attackTime = expf(-1.0f / (attack * 0.001f * sampleRate));
    float releaseTime = expf(-1.0f / (release * 0.001f * sampleRate));

    // Update envelope
    if (inputAbs > state[channel].envelope) {
        state[channel].envelope = attackTime * state[channel].envelope +
                                (1.0f - attackTime) * inputAbs;
    } else {
        state[channel].envelope = releaseTime * state[channel].envelope +
                                (1.0f - releaseTime) * inputAbs;
    }

    // Scale envelope by sensitivity
    float envelopeScaled = state[channel].envelope * sensitivity;

    // Calculate filter frequency
    float baseFreq = 200.0f;  // Base frequency
    float maxFreq = 3000.0f;  // Maximum frequency
    float freqRange = maxFreq - baseFreq;

    // Scale frequency based on envelope and range
    float modAmount = range * envelopeScaled;
    if (!up) modAmount = range * (1.0f - envelopeScaled); // Invert for down mode

    float filterFreq = baseFreq + modAmount * freqRange;

    // Process state variable filter
    float output = processSVF(input,
                            filterFreq,
                            resonance,
                            &state[channel].filterState1,
                            &state[channel].filterState2,
                            sampleRate);

    // Apply some makeup gain based on resonance
    float makeupGain = 1.0f + (resonance * 2.0f);
    output *= makeupGain;

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
