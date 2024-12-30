// Moog-style resonant LPF

#include "null-unit.h"

static NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 2
#define PARAM_CUTOFF 0
#define PARAM_RESONANCE 1

// Filter state
static float p0[2] = {0.0f, 0.0f};
static float p1[2] = {0.0f, 0.0f};
static float p2[2] = {0.0f, 0.0f};
static float p3[2] = {0.0f, 0.0f};

int main(int argc, char *argv[]) {
  NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo) {
    .name= "mooglpf",
    .channelsIn = 1,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  // Cutoff parameter (0.0 to 127.0: midi note)
  gen_midi_float("cutoff", &unitInfo.params[PARAM_CUTOFF]);

  // Resonance parameter (0.0 to 1.0)
  gen_midi_float("resonance", &unitInfo.params[PARAM_RESONANCE]);
  unitInfo.params[PARAM_RESONANCE].max.f = 1.0f;

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Input validation
    if (isnan(input)) return 0.0f;
    if (isinf(input)) return 0.0f;

    float cutoff = noteToFreq(unitInfo.params[PARAM_CUTOFF].value.f);
    float resonance = unitInfo.params[PARAM_RESONANCE].value.f;

    // Ensure cutoff frequency is valid
    if (cutoff <= 0.0f || isnan(cutoff)) {
        cutoff = 440.0f; // fallback to a reasonable default
    }

    // Constrain cutoff and resonance
    cutoff = cutoff > sampleRate/2.0f ? sampleRate/2.0f : cutoff;
    resonance = resonance > 1.0f ? 1.0f : (resonance < 0.0f ? 0.0f : resonance);

    // Calculate filter coefficients
    float f = 2.0f * cutoff / sampleRate; // normalized frequency
    f = f > 0.99f ? 0.99f : f; // Limit maximum frequency

    float k = 4.0f * (resonance); // Resonance parameter
    float p = f * (1.8f - 0.8f * f); // Empirical tuning
    float scale = 1.0f - p;

    // Ensure channel index is valid
    channel = channel % 2;

    // Apply the filter (four cascaded one-pole filters)
    float feedback = p3[channel] * k;
    float t0 = p0[channel];
    float t1 = p1[channel];
    float t2 = p2[channel];
    float t3 = p3[channel];

    // Calculate new values
    p0[channel] += p * (scale * input - feedback - p0[channel]);
    p1[channel] += p * (p0[channel] - p1[channel]);
    p2[channel] += p * (p1[channel] - p2[channel]);
    p3[channel] += p * (p2[channel] - p3[channel]);

    float output = p3[channel];

    // Soft clipping
    if (output > 1.0f) output = 1.0f;
    if (output < -1.0f) output = -1.0f;

    // Ensure output is valid
    if (isnan(output)) output = 0.0f;
    if (isinf(output)) output = 0.0f;

    return output;
}

// Get info about the unit
NullUnitnInfo* get_info() {
  return &unitInfo;
}

// set a parameter
void param_set(uint8_t paramId, NullUnitParamValue* value) {
  if (paramId >= PARAM_COUNT) {
    return;
  }
  unitInfo.params[paramId].value = *value;
}

// get the current value of a parameter
NullUnitParamValue* param_get(uint8_t paramId) {
  if (paramId >= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}
