// simple example with no params, that just copies input to output

#include "null-unit.h"

// called when the unit is loaded, returns the number of params it accepts
void init(unsigned int initialParams[]) {
}

// returns the name of the unit (32 characters, max)
char* get_name_unit() {
  return "copy";
}

// get param count
unsigned int get_param_count() {
  return 0;
}

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param) {
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
  return input;
}

// set a parameter
void param_set(unsigned int param, unsigned int value) {
}

// get the current value of a parameter
unsigned int param_get(unsigned int param) {
  return 0;
}
