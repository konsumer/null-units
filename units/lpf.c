// simple LPF

#include "null-unit.h"

static NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 1
#define PARAM_CUTOFF 0

// called when the unit is loaded, returns the number of params it accepts
int main(int argc, char *argv[]) {
  NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo) {
    .name= "lpf",
    .channelsIn = 1,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  gen_midi_float("cutoff", &unitInfo.params[PARAM_CUTOFF]);
  return 0;
}

// called when you plugin is unloaded
void destroy() {}

static float lastOutput[2] = {0.0f, 0.0f}; // For stereo, one per channel

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float cutoffFrequency = noteToFreq(unitInfo.params[PARAM_CUTOFF].value.f);

    // Ensure cutoff frequency is valid
    if (cutoffFrequency <= 0.0f || isnan(cutoffFrequency)) {
        cutoffFrequency = 440.0f; // fallback to a reasonable default
    }

    // Calculate alpha (smoothing factor)
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoffFrequency);
    float alpha = dt / (rc + dt);

    // Ensure alpha stays between 0 and 1
    alpha = alpha > 1.0f ? 1.0f : (alpha < 0.0f ? 0.0f : alpha);

    // Ensure channel index is valid
    channel = channel % 2; // Limit to 2 channels

    // Apply the filter
    lastOutput[channel] = lastOutput[channel] + alpha * (input - lastOutput[channel]);

    // Ensure output is valid
    if (isnan(lastOutput[channel])) lastOutput[channel] = 0.0f;
    if (isinf(lastOutput[channel])) lastOutput[channel] = 0.0f;

    return lastOutput[channel];
}

// Get info about the unit
NullUnitnInfo* get_info() {
  return &unitInfo;
}

// set a parameter
void param_set(uint8_t paramId, NullUnitParamValue* value) {
  if (paramId>= PARAM_COUNT) {
    return;
  }
  unitInfo.params[paramId].value = *value;
  // show_info();
}

// get the current value of a parameter
NullUnitParamValue* param_get(uint8_t paramId) {
  if (paramId>= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}
