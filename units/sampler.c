// simple looping sampler null-unit
// that can be used as a wave-table oscillator
// eventually, I will embed wav-tables
// but I am testing loading sample-data from host

#include "null-unit.h"

// params: pitch, sampleID
unsigned int params[2];

// this holds the same loaded from host (0-255 position float)
unsigned int sample[1024] = {};


// called when the unit is loaded
void init(unsigned int initialParams[2]) {
  if (initialParams != NULL) {
    params[0] = initialParams[0];
    params[1] = initialParams[1];
  } else {
    params[0] = 440;
    params[1] = 0;
  }
  get_bytes(params[1], 0, 1024, sample);

  show_params(params);
}

// returns the name of the unit (32 characters, max)
char* get_name_unit() {
  return "sampler";
}

// get param count
unsigned int get_param_count() {
  return sizeof(params) / sizeof(int);
}

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param) {
  switch(param) {
    case 0: return "pitch";
    case 1: return "sample ID";
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
    float scaledPos = (position * 4.0f * params[0]) / 440.0f;
    unsigned int samplePos = (unsigned int)scaledPos % 1024;
    char o[20];
    itoa(sample[samplePos], o);
    trace(o);
    return *((float*)&sample[samplePos]);
}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
  if (param > 2) {
    return;
  }
  params[param] = value;
  if (param == 1 && value != params[1]) {
    get_bytes(params[1], 0, 1024, sample);
  }
  show_params(params);
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  if (param > 2) {
    return 0;
  }
  return params[param];
}
