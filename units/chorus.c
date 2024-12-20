// Chorus effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 4
#define PARAM_RATE 0     // LFO rate
#define PARAM_DEPTH 1    // Effect depth
#define PARAM_MIX 2      // Wet/dry mix
#define PARAM_VOICES 3   // Number of voices (1-3)

#define MAX_VOICES 3
#define MAX_DELAY 4096   // Maximum delay buffer size (~93ms @ 44.1kHz)

typedef struct {
    float* buffer;
    int writeIndex;
    float lfoPhase;
} DelayLine;

static DelayLine delays[MAX_VOICES][2];  // For stereo, multiple voices

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "chorus",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Rate parameter (0.1 to 3 Hz)
    unitInfo.params[PARAM_RATE] = (NullUnitParamInfo) {
        .name = "rate",
        .value = {.f = 0.8f},
        .min = {.f = 0.1f},
        .max = {.f = 3.0f},
        .type = NULL_PARAM_F32
    };

    // Depth parameter (0.0 to 1.0)
    unitInfo.params[PARAM_DEPTH] = (NullUnitParamInfo) {
        .name = "depth",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Mix parameter (0.0 to 1.0)
    unitInfo.params[PARAM_MIX] = (NullUnitParamInfo) {
        .name = "mix",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Voices parameter (1 to 3)
    unitInfo.params[PARAM_VOICES] = (NullUnitParamInfo) {
        .name = "voices",
        .value = {.i = 2},
        .min = {.i = 1},
        .max = {.i = 3},
        .type = NULL_PARAM_I32
    };

    // Initialize delay lines
    for (int v = 0; v < MAX_VOICES; v++) {
        for (int ch = 0; ch < 2; ch++) {
            delays[v][ch].buffer = (float*)calloc(MAX_DELAY, sizeof(float));
            delays[v][ch].writeIndex = 0;
            delays[v][ch].lfoPhase = v * (2.0f * M_PI / MAX_VOICES); // Spread initial phases
        }
    }

    return 0;
}

void destroy() {
    for (int v = 0; v < MAX_VOICES; v++) {
        for (int ch = 0; ch < 2; ch++) {
            free(delays[v][ch].buffer);
        }
    }
}

// Helper function to read from delay line with linear interpolation
float readDelay(DelayLine* dl, float delay) {
    float readPos = dl->writeIndex - delay;
    while (readPos < 0.0f) readPos += MAX_DELAY;

    int readIndex = (int)readPos;
    float frac = readPos - readIndex;
    int nextIndex = (readIndex + 1) % MAX_DELAY;

    return dl->buffer[readIndex] * (1.0f - frac) + dl->buffer[nextIndex] * frac;
}

// Helper function to write to delay line
void writeDelay(DelayLine* dl, float value) {
    dl->buffer[dl->writeIndex] = value;
    dl->writeIndex = (dl->writeIndex + 1) % MAX_DELAY;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float rate = unitInfo.params[PARAM_RATE].value.f;
    float depth = unitInfo.params[PARAM_DEPTH].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;
    int voices = unitInfo.params[PARAM_VOICES].value.i;

    // Parameter validation
    rate = rate > 3.0f ? 3.0f : (rate < 0.1f ? 0.1f : rate);
    depth = depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);
    voices = voices > MAX_VOICES ? MAX_VOICES : (voices < 1 ? 1 : voices);

    // Ensure channel index is valid
    channel = channel % 2;

    float wetSignal = 0.0f;
    float baseDelay = 20.0f; // Base delay in ms

    // Process each voice
    for (int v = 0; v < voices; v++) {
        DelayLine* dl = &delays[v][channel];

        // Update LFO phase for this voice
        dl->lfoPhase += 2.0f * M_PI * rate / sampleRate;
        if (dl->lfoPhase >= 2.0f * M_PI) {
            dl->lfoPhase -= 2.0f * M_PI;
        }

        // Calculate LFO value (sine wave with slight randomization per voice)
        float lfoValue = sinf(dl->lfoPhase + (v * 0.5f));

        // Calculate delay time in samples
        float baseDelaySamples = (baseDelay * sampleRate) / 1000.0f;
        float modulationDepth = depth * 10.0f; // Max 10ms of modulation
        float currentDelay = baseDelaySamples +
                           ((lfoValue * 0.5f + 0.5f) * modulationDepth * sampleRate / 1000.0f);

        // Ensure delay stays within buffer size
        if (currentDelay >= MAX_DELAY - 1) currentDelay = MAX_DELAY - 1;
        if (currentDelay < 0) currentDelay = 0;

        // Read from delay line and accumulate
        wetSignal += readDelay(dl, currentDelay) / voices;

        // Write to delay line
        writeDelay(dl, input);
    }

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (wetSignal * mix);

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
