// Tremolo effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 5
#define PARAM_RATE 0      // LFO rate
#define PARAM_DEPTH 1     // Effect depth
#define PARAM_WAVEFORM 2  // LFO waveform type
#define PARAM_PHASE 3     // Stereo phase offset
#define PARAM_SYNC 4      // Tempo sync

// Waveform types
#define WAVE_SINE 0
#define WAVE_SQUARE 1
#define WAVE_TRIANGLE 2
#define WAVE_SAW 3

static float phase[2] = {0.0f, 0.0f};  // LFO phase for each channel

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "tremolo",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Rate parameter (0.1 to 20.0 Hz)
    unitInfo.params[PARAM_RATE] = (NullUnitParamInfo) {
        .name = "rate",
        .value = {.f = 5.0f},
        .min = {.f = 0.1f},
        .max = {.f = 20.0f},
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

    // Waveform parameter (0 to 3)
    unitInfo.params[PARAM_WAVEFORM] = (NullUnitParamInfo) {
        .name = "waveform",
        .value = {.i = WAVE_SINE},
        .min = {.i = 0},
        .max = {.i = 3},
        .type = NULL_PARAM_I32
    };

    // Phase parameter (0.0 to 360.0 degrees)
    unitInfo.params[PARAM_PHASE] = (NullUnitParamInfo) {
        .name = "phase",
        .value = {.f = 0.0f},
        .min = {.f = 0.0f},
        .max = {.f = 360.0f},
        .type = NULL_PARAM_F32
    };

    // Sync parameter (0 or 1)
    unitInfo.params[PARAM_SYNC] = (NullUnitParamInfo) {
        .name = "sync",
        .value = {.i = false},
        .min = {.i = false},
        .max = {.i = true},
        .type = NULL_PARAM_BOOL
    };

    return 0;
}

void destroy() {}

// Helper function for triangle wave
float triangleWave(float phase) {
    float normPhase = phase / (2.0f * M_PI);
    return 2.0f * fabsf(2.0f * (normPhase - floorf(normPhase + 0.5f))) - 1.0f;
}

// Helper function for saw wave
float sawWave(float phase) {
    float normPhase = phase / (2.0f * M_PI);
    return 2.0f * (normPhase - floorf(normPhase + 0.5f));
}

// Helper function for square wave with anti-aliasing
float squareWave(float phase) {
    float sine = sinf(phase);
    return sine > 0.0f ? 1.0f : -1.0f;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float rate = unitInfo.params[PARAM_RATE].value.f;
    float depth = unitInfo.params[PARAM_DEPTH].value.f;
    int waveform = unitInfo.params[PARAM_WAVEFORM].value.i;
    float phaseOffset = unitInfo.params[PARAM_PHASE].value.f;
    int sync = unitInfo.params[PARAM_SYNC].value.i;

    // Parameter validation
    rate = rate > 20.0f ? 20.0f : (rate < 0.1f ? 0.1f : rate);
    depth = depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth);
    waveform = waveform > 3 ? 3 : (waveform < 0 ? 0 : waveform);
    phaseOffset = phaseOffset > 360.0f ? 360.0f : (phaseOffset < 0.0f ? 0.0f : phaseOffset);

    // Ensure channel index is valid
    channel = channel % 2;

    // Calculate base phase
    float phaseIncrement = 2.0f * M_PI * rate / sampleRate;

    // Add phase offset for second channel
    float currentPhase = phase[channel];
    if (channel == 1) {
        currentPhase += (phaseOffset / 360.0f) * 2.0f * M_PI;
    }

    // Update phase
    phase[channel] = phase[channel] + phaseIncrement;
    if (phase[channel] >= 2.0f * M_PI) {
        phase[channel] -= 2.0f * M_PI;
    }

    // Calculate modulation value based on waveform
    float modulation;
    switch(waveform) {
        case WAVE_SQUARE:
            modulation = squareWave(currentPhase);
            break;
        case WAVE_TRIANGLE:
            modulation = triangleWave(currentPhase);
            break;
        case WAVE_SAW:
            modulation = sawWave(currentPhase);
            break;
        case WAVE_SINE:
        default:
            modulation = sinf(currentPhase);
            break;
    }

    // Convert modulation to unipolar (0 to 1) and scale by depth
    float amplitude = 1.0f - (depth * (modulation * 0.5f + 0.5f));

    // Apply amplitude modulation
    float output = input * amplitude;

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
