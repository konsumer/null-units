// Wavefolder effect

#include "null-unit.h"
#include <math.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 5
#define PARAM_GAIN 0      // Input gain (drive amount)
#define PARAM_FOLDS 1     // Number of folds
#define PARAM_TYPE 2      // Folding algorithm type
#define PARAM_BIAS 3      // DC bias for asymmetric folding
#define PARAM_MIX 4       // Wet/dry mix

// Folding types
#define FOLD_SINE 0       // Sine-based folder
#define FOLD_TRI 1        // Triangle-based folder
#define FOLD_CLIP 2       // Hard clip folder
#define FOLD_ASYM 3       // Asymmetric folder

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "wavefolder",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Gain parameter (1.0 to 50.0)
    unitInfo.params[PARAM_GAIN] = (NullUnitParamInfo) {
        .name = "gain",
        .value = {.f = 5.0f},
        .min = {.f = 1.0f},
        .max = {.f = 50.0f},
        .type = NULL_PARAM_F32
    };

    // Folds parameter (1.0 to 10.0)
    unitInfo.params[PARAM_FOLDS] = (NullUnitParamInfo) {
        .name = "folds",
        .value = {.f = 2.0f},
        .min = {.f = 1.0f},
        .max = {.f = 10.0f},
        .type = NULL_PARAM_F32
    };

    // Type parameter (0 to 3)
    unitInfo.params[PARAM_TYPE] = (NullUnitParamInfo) {
        .name = "type",
        .value = {.i = FOLD_SINE},
        .min = {.i = 0},
        .max = {.i = 3},
        .type = NULL_PARAM_I32
    };

    // Bias parameter (-1.0 to 1.0)
    unitInfo.params[PARAM_BIAS] = (NullUnitParamInfo) {
        .name = "bias",
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

// Sine folder
float sineFold(float input, float folds) {
    return sinf(input * M_PI * folds);
}

// Triangle folder
float triFold(float input, float folds) {
    float phase = fmodf(input * folds + 1.0f, 2.0f) - 1.0f;
    return 2.0f * fabsf(phase) - 1.0f;
}

// Hard clip folder
float clipFold(float input, float folds) {
    float folded = input;
    float threshold = 1.0f / folds;

    // Iterative folding
    for (int i = 0; i < (int)folds; i++) {
        if (folded > threshold) {
            folded = 2.0f * threshold - folded;
        } else if (folded < -threshold) {
            folded = -2.0f * threshold - folded;
        }
    }

    return folded;
}

// Asymmetric folder
float asymFold(float input, float folds, float bias) {
    float folded = input + bias;
    float pos_thresh = (1.0f + bias) / folds;
    float neg_thresh = (-1.0f + bias) / folds;

    // Iterative asymmetric folding
    for (int i = 0; i < (int)folds; i++) {
        if (folded > pos_thresh) {
            folded = 2.0f * pos_thresh - folded;
        } else if (folded < neg_thresh) {
            folded = 2.0f * neg_thresh - folded;
        }
    }

    return folded - bias;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float gain = unitInfo.params[PARAM_GAIN].value.f;
    float folds = unitInfo.params[PARAM_FOLDS].value.f;
    int type = unitInfo.params[PARAM_TYPE].value.i;
    float bias = unitInfo.params[PARAM_BIAS].value.f;
    float mix = unitInfo.params[PARAM_MIX].value.f;

    // Parameter validation
    gain = gain > 50.0f ? 50.0f : (gain < 1.0f ? 1.0f : gain);
    folds = folds > 10.0f ? 10.0f : (folds < 1.0f ? 1.0f : folds);
    type = type > 3 ? 3 : (type < 0 ? 0 : type);
    bias = bias > 1.0f ? 1.0f : (bias < -1.0f ? -1.0f : bias);
    mix = mix > 1.0f ? 1.0f : (mix < 0.0f ? 0.0f : mix);

    // Apply input gain
    float driven = input * gain;

    // Apply folding based on type
    float folded;
    switch(type) {
        case FOLD_TRI:
            folded = triFold(driven, folds);
            break;
        case FOLD_CLIP:
            folded = clipFold(driven, folds);
            break;
        case FOLD_ASYM:
            folded = asymFold(driven, folds, bias);
            break;
        case FOLD_SINE:
        default:
            folded = sineFold(driven, folds);
            break;
    }

    // Gain compensation
    folded *= 1.0f / (1.0f + (gain * 0.1f));

    // Mix dry and wet signals
    float output = (input * (1.0f - mix)) + (folded * mix);

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
