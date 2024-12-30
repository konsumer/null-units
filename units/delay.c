// Digital Delay/Echo

#include "null-unit.h"
#include <stdlib.h>
#include <string.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 4
#define PARAM_TIME 0    // Delay time
#define PARAM_FEEDBACK 1 // Feedback amount
#define PARAM_MIX 2     // Wet/dry mix
#define PARAM_SYNC 3    // Tempo sync on/off

#define MAX_DELAY 96000 // 2 seconds at 48kHz

typedef struct {
    float* buffer;
    int writeIndex;
    int length;
} DelayLine;

static DelayLine delays[2]; // Stereo delay lines

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "delay",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Time parameter (0.0 to 2000.0 ms)
    unitInfo.params[PARAM_TIME] = (NullUnitParamInfo) {
        .name = "time",
        .value = {.f = 500.0f},
        .min = {.f = 0.0f},
        .max = {.f = 2000.0f},
        .type = NULL_PARAM_F32
    };

    // Feedback parameter (0.0 to 1.0)
    unitInfo.params[PARAM_FEEDBACK] = (NullUnitParamInfo) {
        .name = "feedback",
        .value = {.f = 0.3f},
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

    // Sync parameter (0 or 1)
    gen_bool("sync", &unitInfo.params[PARAM_SYNC]);

    // Initialize delay lines
    for (int ch = 0; ch < 2; ch++) {
        delays[ch].buffer = (float*)calloc(MAX_DELAY, sizeof(float));
        delays[ch].writeIndex = 0;
        delays[ch].length = MAX_DELAY;
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
float readDelay(DelayLine* dl, float delayTime, float sampleRate) {
    float delayInSamples = delayTime * sampleRate / 1000.0f;
    int readIndex = dl->writeIndex - (int)delayInSamples;
    float fraction = delayInSamples - (int)delayInSamples;

    // Wrap around
    while (readIndex < 0) readIndex += dl->length;
    int nextIndex = (readIndex + 1) % dl->length;

    // Linear interpolation
    return dl->buffer[readIndex] * (1.0f - fraction) +
           dl->buffer[nextIndex] * fraction;
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

    float delayTime = unitInfo.params[PARAM_TIME].value.f;
    float feedback = unitInfo.params[PARAM_FEEDBACK].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;
    int sync = unitInfo.params[PARAM_SYNC].value.i;

    // Parameter validation
    delayTime = delayTime > 2000.0f ? 2000.0f : (delayTime < 0.0f ? 0.0f : delayTime);
    feedback = feedback > 1.0f ? 1.0f : (feedback < 0.0f ? 0.0f : feedback);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    // If sync is enabled, quantize delay time to musical divisions
    if (sync) {
        // Assuming 120 BPM as default tempo
        float beatLength = 60000.0f / 120.0f; // Length of one beat in ms
        float divisions[] = {1.0f, 0.75f, 0.5f, 0.375f, 0.25f, 0.125f}; // Whole, dotted half, half, dotted quarter, quarter, eighth

        // Find closest division
        float minDiff = beatLength;
        float targetTime = beatLength;
        for (int i = 0; i < 6; i++) {
            float divTime = beatLength * divisions[i];
            float diff = fabsf(delayTime - divTime);
            if (diff < minDiff) {
                minDiff = diff;
                targetTime = divTime;
            }
        }
        delayTime = targetTime;
    }

    // Read from delay line
    float delayed = readDelay(&delays[channel], delayTime, sampleRate);

    // Calculate new sample with feedback
    float newSample = input + (delayed * feedback);

    // Apply soft clipping to prevent runaway feedback
    if (newSample > 1.0f) newSample = 1.0f;
    if (newSample < -1.0f) newSample = -1.0f;

    // Write to delay line
    writeDelay(&delays[channel], newSample);

    // Mix dry and wet signals
    float output = input * (1.0f - mix) + delayed * mix;

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
