// Flanger effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 4
#define PARAM_RATE 0     // LFO rate
#define PARAM_DEPTH 1    // Effect depth
#define PARAM_DELAY 2    // Base delay time
#define PARAM_FEEDBACK 3 // Feedback amount

#define MAX_DELAY 2048   // Maximum delay buffer size

typedef struct {
    float* buffer;
    int writeIndex;
} DelayLine;

static DelayLine delays[2];  // For stereo
static float lfoPhase[2] = {0.0f};  // LFO phase for each channel

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "flanger",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Rate parameter (0.1 to 10 Hz)
    unitInfo.params[PARAM_RATE] = (NullUnitParamInfo) {
        .name = "rate",
        .value = {.f = 0.5f},
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

    // Delay parameter (1.0 to 10.0 ms)
    unitInfo.params[PARAM_DELAY] = (NullUnitParamInfo) {
        .name = "delay",
        .value = {.f = 3.0f},
        .min = {.f = 1.0f},
        .max = {.f = 10.0f},
        .type = NULL_PARAM_F32
    };

    // Feedback parameter (-0.95 to 0.95)
    unitInfo.params[PARAM_FEEDBACK] = (NullUnitParamInfo) {
        .name = "feedback",
        .value = {.f = 0.3f},
        .min = {.f = -0.95f},
        .max = {.f = 0.95f},
        .type = NULL_PARAM_F32
    };

    // Initialize delay lines
    for (int ch = 0; ch < 2; ch++) {
        delays[ch].buffer = (float*)calloc(MAX_DELAY, sizeof(float));
        delays[ch].writeIndex = 0;
    }

    return 0;
}

void destroy() {
    // Free delay buffers
    for (int ch = 0; ch < 2; ch++) {
        free(delays[ch].buffer);
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
    float baseDelay = unitInfo.params[PARAM_DELAY].value.f;
    float feedback = unitInfo.params[PARAM_FEEDBACK].value.f;

    // Parameter validation
    rate = rate > 10.0f ? 10.0f : (rate < 0.1f ? 0.1f : rate);
    depth = depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth);
    baseDelay = baseDelay > 10.0f ? 10.0f : (baseDelay < 1.0f ? 1.0f : baseDelay);
    feedback = feedback > 0.95f ? 0.95f : (feedback < -0.95f ? -0.95f : feedback);

    // Ensure channel index is valid
    channel = channel % 2;

    // Update LFO phase
    lfoPhase[channel] += 2.0f * M_PI * rate / sampleRate;
    if (lfoPhase[channel] >= 2.0f * M_PI) {
        lfoPhase[channel] -= 2.0f * M_PI;
    }

    // Calculate LFO value (sine wave)
    float lfoValue = sinf(lfoPhase[channel]);

    // Calculate delay time in samples
    float baseDelaySamples = (baseDelay * sampleRate) / 1000.0f; // Convert ms to samples
    float modulationDepth = depth * baseDelaySamples;
    float currentDelay = baseDelaySamples + (lfoValue * modulationDepth);

    // Ensure delay stays within buffer size
    if (currentDelay >= MAX_DELAY - 1) currentDelay = MAX_DELAY - 1;
    if (currentDelay < 0) currentDelay = 0;

    // Read from delay line
    float delayedSample = readDelay(&delays[channel], currentDelay);

    // Calculate new sample with feedback
    float newSample = input + (delayedSample * feedback);

    // Write to delay line
    writeDelay(&delays[channel], newSample);

    // Mix delayed and dry signals
    float output = input + delayedSample;
    output *= 0.707f; // Reduce output by -3dB to prevent clipping

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
