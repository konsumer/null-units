// simple LPF

#include "null-unit.h"

#define PARAM_GAIN 0
#define PARAM_CUTOFF 1
#define PARAM_RESONANCE 2
#define PARAM_LAST 2
static unsigned int params[3];

void init() {
  // disable printf buffering
  setvbuf(stdout, NULL, _IONBF, 0);
}

float lastOutput = 0.0f;
float momentum = 0.0f;

float doResonantLPF(float input, float cutoff, float resonance) {
    // Convert cutoff frequency to coefficient (0 to 1)
    float freq_coeff = 2.0f * sinf(M_PI * cutoff / SAMPLE_RATE);
    if (freq_coeff > 0.99f) freq_coeff = 0.99f;

    // Convert resonance from 0-100 to useful range (0 to ~0.99)
    float res_coeff = resonance / 101.0f;

    float distanceToGo = input - lastOutput;
    momentum = momentum * res_coeff + distanceToGo * freq_coeff;
    lastOutput += momentum + distanceToGo * freq_coeff;

    return lastOutput;
}

float process(unsigned char position, float input, unsigned char channel) {
  return doResonantLPF(input, (float)params[PARAM_CUTOFF], (float)params[PARAM_RESONANCE]) * ((float)params[PARAM_GAIN] / 100.0f);
}

// returns the name of the unit (32 characters, max)
char* get_name_unit() {
  return "lpf";
}

// get param count
unsigned int get_param_count() {
  return PARAM_LAST + 1;
}

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param) {
  switch(param) {
    case PARAM_GAIN: return "gain";
    case PARAM_CUTOFF: return "cutoff";
    case PARAM_RESONANCE: return "resonance";
    default: return NULL;
  }
}

// returns number of channels
unsigned char get_channel_count() {
  return 1;
}

// called when you plugin is unloaded
void destroy() {}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
  if (param > PARAM_LAST) {
    return;
  }
  params[param] = value;
  show_params(params);
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  if (param > PARAM_LAST) {
    return 0;
  }
  return params[param];
}
