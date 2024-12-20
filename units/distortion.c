// Distortion/Overdrive effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 5
#define PARAM_DRIVE 0    // Amount of drive/gain
#define PARAM_TONE 1     // Tone control (low-pass filter)
#define PARAM_TYPE 2     // Distortion type
#define PARAM_CRUNCH 3   // Additional harmonics/crunch
#define PARAM_MIX 4      // Wet/dry mix

// Distortion types
#define DIST_SOFT 0      // Soft clip (tube-like)
#define DIST_HARD 1      // Hard clip
#define DIST_FUZZ 2      // Fuzz (asymmetric)
#define DIST_FOLD 3      // Wavefolder
#define DIST_CRUSH 4     // Digital crush

// State variables
static float lastSample[2] = {0.0f}; // For tone control
static float dc_blocker[2] = {0.0f}; // DC offset removal

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "distortion",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Drive parameter (1.0 to 100.0)
    unitInfo.params[PARAM_DRIVE] = (NullUnitParamInfo) {
        .name = "drive",
        .value = {.f = 10.0f},
        .min = {.f = 1.0f},
        .max = {.f = 100.0f},
        .type = NULL_PARAM_F32
    };

    // Tone parameter (0.0 to 1.0)
    unitInfo.params[PARAM_TONE] = (NullUnitParamInfo) {
        .name = "tone",
        .value = {.f = 0.5f},
        .min = {.f = 0.0f},
        .max = {.f = 1.0f},
        .type = NULL_PARAM_F32
    };

    // Type parameter (0 to 4)
    unitInfo.params[PARAM_TYPE] = (NullUnitParamInfo) {
        .name = "type",
        .value = {.i = DIST_SOFT},
        .min = {.i = 0},
        .max = {.i = 4},
        .type = NULL_PARAM_I32
    };

    // Crunch parameter (0.0 to 1.0)
    unitInfo.params[PARAM_CRUNCH] = (NullUnitParamInfo) {
        .name = "crunch",
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

// Distortion shape functions
float softClip(float input, float drive) {
    float x = input * drive;
    return (2.0f / M_PI) * atan(x);
}

float hardClip(float input, float drive) {
    float x = input * drive;
    if (x > 1.0f) return 1.0f;
    if (x < -1.0f) return -1.0f;
    return x;
}

float fuzz(float input, float drive) {
    float x = input * drive;
    float sign = x > 0 ? 1.0f : -1.0f;
    return sign * (1.0f - expf(-fabsf(x)));
}

float waveFolder(float input, float drive) {
    float x = input * drive;
    x = fmodf(x + 1.0f, 4.0f) - 2.0f;
    return x > 1.0f ? 2.0f - x : (x < -1.0f ? -2.0f - x : x);
}

float bitCrush(float input, float drive) {
    float x = input * drive;
    float bits = 16.0f - (drive * 12.0f); // Reduce bit depth with drive
    float scale = powf(2.0f, bits - 1.0f);
    return floorf(x * scale) / scale;
}

// DC Blocker
float dcBlock(float input, float *lastDC, float coeff) {
    float output = input - *lastDC;
    *lastDC = input - (output * coeff);
    return output;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float drive = unitInfo.params[PARAM_DRIVE].value.f;
    float tone = unitInfo.params[PARAM_TONE].value.f;
    int type = unitInfo.params[PARAM_TYPE].value.i;
    float crunch = unitInfo.params[PARAM_CRUNCH].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Parameter validation
    drive = drive > 100.0f ? 100.0f : (drive < 1.0f ? 1.0f : drive);
    tone = tone > 1.0f ? 1.0f : (tone < 0.0f ? 0.0f : tone);
    type = type > 4 ? 4 : (type < 0 ? 0 : type);
    crunch = crunch > 1.0f ? 1.0f : (crunch < 0.0f ? 0.0f : crunch);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Ensure channel index is valid
    channel = channel % 2;

    // Pre-gain and add crunch
    float preGain = 1.0f + (crunch * 2.0f);
    input *= preGain;

    // Add some even harmonics for crunch
    if (crunch > 0.0f) {
        input += (input * input * input) * crunch * 0.3f;
    }

    // Apply distortion based on type
    float distorted;
    switch(type) {
        case DIST_HARD:
            distorted = hardClip(input, drive);
            break;
        case DIST_FUZZ:
            distorted = fuzz(input, drive);
            break;
        case DIST_FOLD:
            distorted = waveFolder(input, drive);
            break;
        case DIST_CRUSH:
            distorted = bitCrush(input, drive);
            break;
        case DIST_SOFT:
        default:
            distorted = softClip(input, drive);
            break;
    }

    // Apply tone control (low-pass filter)
    float toneFreq = 200.0f + (tone * 15000.0f); // 200Hz to 15kHz
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * toneFreq);
    float alpha = dt / (rc + dt);

    distorted = lastSample[channel] + (alpha * (distorted - lastSample[channel]));
    lastSample[channel] = distorted;

    // Remove DC offset
    distorted = dcBlock(distorted, &dc_blocker[channel], 0.995f);

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (distorted * mix);

    // Output gain compensation
    output *= 1.0f / (1.0f + (drive * 0.1f));

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
