// simple looping sampler null-unit
// that can be used as a wave-table oscillator
// eventually, I will embed wav-tables
// but I am testing loading sample-data from host

#include "null-unit.h"

#define PARAM_PITCH 0
#define PARAM_SAMPLE 1
#define PARAM_LAST 1

unsigned int params[2];

// this holds the same loaded from host (0-255 position float)
unsigned int sample[1024] = {};


// called when the unit is loaded
void init(unsigned int initialParams[2]) {
  if (initialParams != NULL) {
    params[PARAM_PITCH] = initialParams[PARAM_PITCH];
    params[PARAM_SAMPLE] = initialParams[PARAM_SAMPLE];
  } else {
    params[PARAM_PITCH] = 440;
    params[PARAM_SAMPLE] = 0;
  }
  get_bytes(params[PARAM_SAMPLE], 0, 1024, sample);
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
    case PARAM_PITCH: return "pitch";
    case PARAM_SAMPLE: return "sample ID";
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
    float scaledPos = (position * 4.0f * params[PARAM_PITCH]) / 440.0f;
    unsigned int samplePos = (unsigned int)scaledPos % 1024;

    char o[20];
    itoa(sample[samplePos], o);
    trace(o);

    return *((float*)&sample[samplePos]);
}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
  if (param > PARAM_LAST) {
    return;
  }
  params[param] = value;

  // load sample form host if it changed
  if (param == PARAM_SAMPLE && value != params[PARAM_SAMPLE]) {
    get_bytes(params[PARAM_SAMPLE], 0, 1024, sample);
  }

  show_params(params);
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  if (param > PARAM_LAST) {
    return 0;
  }
  return params[param];
}
