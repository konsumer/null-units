// simple oscillator null-unit

#include "null-unit.h"


// params: pitch, type (sine, square, sawtooth, triangle)
unsigned int params[2];

// called when the unit is loaded, returns the number of params it accepts
unsigned int init() {
  return 2;
}

// returns the name of the unit (32 characters, max)
char* get_name_unit() {
  return "oscillator";
}

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param) {
  switch(param) {
    case 0: return "pitch";
    case 1: return "type"
    default: return NULL;
  }
}

// returns number of channels
unsigned char get_channel_count() {
  return 1;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(unsigned char position, float input, unsigned char channel) {
  return 0.0f;
}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
  if (param > 2) {
    return;
  }
  params[param] = value;
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  if (param > 2) {
    return 0;
  }
  return params[param];
}

