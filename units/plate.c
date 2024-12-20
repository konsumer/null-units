#include "null-unit.h"
#include <stdlib.h>
#include <string.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 3
#define PARAM_SIZE 0    // Room size/decay
#define PARAM_DAMP 1    // High frequency damping
#define PARAM_MIX 2     // Wet/dry mix

#define NUM_DELAYS 4    // Number of parallel delay lines

typedef struct {
    float* buffer;
    int writeIndex;
    int length;
} DelayLine;

static DelayLine delays[NUM_DELAYS][2]; // For stereo, arrays for each channel
static float lastOutput[2] = {0.0f, 0.0f};

// Prime numbers for delay lengths to create dense reverb
static const int delayLengths[NUM_DELAYS] = {1087, 1283, 1511, 1693};

// Initialize delay lines
void initDelayLine(DelayLine* dl, int length) {
    dl->buffer = (float*)calloc(length, sizeof(float));
    dl->writeIndex = 0;
    dl->length = length;
}

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "plate",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Size parameter (0.0 to 1.0)
    unitInfo.params[PARAM_SIZE] = (NullUnitParamInfo) {
        .name = "size",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 4.0f},
        .type = NULL_PARAM_F32
    };

    // Damping parameter (0.0 to 1.0)
    unitInfo.params[PARAM_DAMP] = (NullUnitParamInfo) {
        .name = "damp",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Mix parameter (0.0 to 1.0)
    unitInfo.params[PARAM_MIX] = (NullUnitParamInfo) {
        .name = "mix",
        .value = {.f = 0.3f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Initialize delay lines
    for (int i = 0; i < NUM_DELAYS; i++) {
        for (int ch = 0; ch < 2; ch++) {
            initDelayLine(&delays[i][ch], delayLengths[i]);
        }
    }

    return 0;
}

void destroy() {
    // Free delay line buffers
    for (int i = 0; i < NUM_DELAYS; i++) {
        for (int ch = 0; ch < 2; ch++) {
            free(delays[i][ch].buffer);
        }
    }
}

// Helper function to read from delay line
float readDelay(DelayLine* dl, int offset) {
    int readIndex = dl->writeIndex - offset;
    if (readIndex < 0) readIndex += dl->length;
    return dl->buffer[readIndex];
}

// Helper function to write to delay line
void writeDelay(DelayLine* dl, float value) {
    dl->buffer[dl->writeIndex] = value;
    dl->writeIndex = (dl->writeIndex + 1) % dl->length;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float size = unitInfo.params[PARAM_SIZE].value.f;
    float damp = unitInfo.params[PARAM_DAMP].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Ensure parameters are in valid range
    size = size > 1.0f ? 1.0f : (size < 0.0f ? 0.0f : size);
    damp = damp > 1.0f ? 1.0f : (damp < 0.0f ? 0.0f : damp);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    // Calculate reverb
    float feedback = 0.84f + (size * 0.15f); // Feedback amount based on size
    float damping = damp * 0.4f;             // Damping coefficient

    float wet = 0.0f;

    // Process each delay line
    for (int i = 0; i < NUM_DELAYS; i++) {
        DelayLine* dl = &delays[i][channel];

        // Read from delay line
        float delayed = readDelay(dl, delayLengths[i]);

        // Apply damping
        lastOutput[channel] = (1.0f - damping) * delayed + damping * lastOutput[channel];

        // Calculate new sample
        float newSample = input + (lastOutput[channel] * feedback);

        // Write to delay line
        writeDelay(dl, newSample);

        // Accumulate output
        wet += lastOutput[channel];
    }

    // Average the outputs
    wet /= NUM_DELAYS;

    // Apply soft clipping to prevent excessive resonance
    if (wet > 1.0f) wet = 1.0f;
    if (wet < -1.0f) wet = -1.0f;

    // Mix dry and wet signals
    float output = input * (1.0f - mix) + wet * mix;

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
