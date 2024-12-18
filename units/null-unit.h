// use this header in all C null-units

#include "math.h"
#include <stdio.h>

#define SAMPLE_RATE 48000.0f

// these are exposed from host

// get some named bytes (sample, etc) from host
__attribute__((import_module("env"), import_name("get_data_floats")))
void get_data_floats(unsigned int id, unsigned int offset, unsigned int length, float* out);


// these are exposed from a unit

// called when the unit is loaded
__attribute__((export_name("init")))
void init();

// process a single value, in a 0-255 position frame, return output
__attribute__((export_name("process")))
float process(unsigned char position, float input, unsigned char channel);

// called when you plugin is unloaded
__attribute__((export_name("destroy")))
void destroy();

// get param count
__attribute__((export_name("get_param_count")))
unsigned int get_param_count();

// set a parameter
__attribute__((export_name("param_set")))
void param_set(unsigned int param, unsigned int value);

// get the current value of a parameter
__attribute__((export_name("param_get")))
unsigned int param_get(unsigned int param);

// returns the name of the unit (32 characters, max)
__attribute__((export_name("get_name_unit")))
char* get_name_unit();

// returns the name of the parameter (32 characters, max)
__attribute__((export_name("get_param_name")))
char* get_name_param(unsigned int param);

// returns number of channels
__attribute__((export_name("get_channel_count")))
unsigned char get_channel_count();

// debug function to show my current params
void show_params(unsigned int params[]) {
  int p = get_param_count();
  printf("%s (%d params)\n", get_name_unit(), p);
  for (int i=0;i<p;i++) {
    printf("%s: %u\n", get_name_param(i), params[i]);
  }
}
