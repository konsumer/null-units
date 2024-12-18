// simple tremelo

#include "null-unit.h"

#define PARAM_DEPTH 0
#define PARAM_RATE 1

static float params[3];

// called when the unit is loaded, returns the number of params it accepts
void init() {
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
  if (param == 0) return "depth";
  if (param == 1) return "rate";
  return NULL;
}

// returns number of channels
unsigned char get_channel_count() {
  return 1;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(unsigned char position, float input, unsigned char channel) {
  float depth = params[PARAM_DEPTH] / 100.0f;
  float rate = params[PARAM_RATE] / 100.0f;
  float lfo = 0.5f + 0.5f * sin(2.0f * M_PI * position * rate / 256.0f);
  float modulation = 1.0f - (depth * lfo);
  return input * modulation;
}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
  if (param > 2) {
    return;
  }
  params[param] = (float)value;
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  return (unsigned int)params[param];
}
