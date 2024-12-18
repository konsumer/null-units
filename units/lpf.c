// simple LPF

#include "null-unit.h"

#define PARAM_GAIN 0
#define PARAM_FREQUENCY 1
#define PARAM_RESONANCE 2

static float buffer[256];
static unsigned int params[3];

void init() {
}

float process(unsigned char position, float input, unsigned char channel) {
  float gain = params[PARAM_GAIN] / 100.0f;
  float freq = params[PARAM_FREQUENCY];
  float resonance = params[PARAM_RESONANCE] / 100.0f;
  float output = input * gain;
  output += buffer[position] * resonance;
  buffer[position] = output;
  return output;
}

// returns the name of the unit (32 characters, max)
char* get_name_unit() {
  return "copy";
}

// get param count
unsigned int get_param_count() {
  return sizeof(params) / sizeof(int);
}

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param) {
  if (param == 0) return "gain";
  if (param == 1) return "frequency";
  if (param == 2) return "resonance";
  return NULL;
}

// returns number of channels
unsigned char get_channel_count() {
  return 1;
}

// called when you plugin is unloaded
void destroy() {}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  return 0;
}
