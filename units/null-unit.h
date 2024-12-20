// use this header in all C null-units

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifndef TWO_PI
#define TWO_PI 6.283185307179586f
#endif

// these are exposed from host
__attribute__((import_module("env"), import_name("get_data_floats")))
void get_data_floats(unsigned int id, unsigned int offset, unsigned int length, float* out);

// these are exposed from a unit

typedef enum {
  NULL_PARAM_BOOL,  // stored as uint32_t
  NULL_PARAM_U32,
  NULL_PARAM_I32,
  NULL_PARAM_F32
} NullUnitParamType;

typedef union {
  uint32_t u;
  int32_t i;
  float f;
} NullUnitParamValue;

typedef struct {
  NullUnitParamType type;
  NullUnitParamValue min;
  NullUnitParamValue max;
  NullUnitParamValue value;
  char* name;
} NullUnitParamInfo;

typedef struct {
  char* name;
  uint8_t channelsIn;
  uint8_t channelsOut;
  uint8_t paramCount;
  NullUnitParamInfo* params;
} NullUnitnInfo;

__attribute__((export_name("malloc")))
void* _null_unit_malloc(size_t size) {
  return malloc(size);
}

__attribute__((export_name("free")))
void _null_unit_free(void* ptr) {
  free(ptr);
}

__attribute__((export_name("get_info")))
NullUnitnInfo* get_info();

/*
sampleRate: engine sampleRate
currentTime: represents the ever-increasing context time of the audio block being processed.
*/
__attribute__((export_name("process")))
float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime);

__attribute__((export_name("destroy")))
void destroy();

__attribute__((export_name("param_set")))
void param_set(uint8_t paramId, NullUnitParamValue* value);

__attribute__((export_name("param_get")))
NullUnitParamValue* param_get(uint8_t paramId);

// HELPERS

// get the string representation of a single param
void param_string(NullUnitParamInfo param, char* out) {
  switch(param.type) {
    case NULL_PARAM_BOOL:
      snprintf(out, 100, "%s: %s", param.name, param.value.u ? "true" : "false");
      break;
    case NULL_PARAM_U32:
      snprintf(out, 100, "%s: %u", param.name, param.value.u);
      break;
    case NULL_PARAM_I32:
      snprintf(out, 100, "%s: %d", param.name, param.value.i);
      break;
    case NULL_PARAM_F32:
      snprintf(out, 100, "%s: %f", param.name, param.value.f);
      break;
    default:
      snprintf(out, 100, "%s: unknown", param.name);
      break;
  }
}

// debug function to show my current params
void show_info() {
  setvbuf(stdout, NULL, _IONBF, 0);
  NullUnitnInfo* info = get_info();
  printf("%s\nin: %u\nout: %u\nparams: %u\n", info->name, info->channelsIn, info->channelsOut, info->paramCount);
  char p[100];
  for (int i = 0; i < info->paramCount; i++) {
    param_string(info->params[i], p);
    printf("  %s\n", p);
  }
}

// f32 midi note num to frequency
float noteToFreq(float note) {
  float a = 440.0f; // frequency of A4 (MIDI note 69)
  return a * powf(2.0f, (note - 69.0f) / 12.0f);
}

// f32 frequency to midi note
float freqToNote(float freq) {
  return 69.0f + 12.0f * log2f(freq / 440.0f);
}

// initialize a f32 0-127 (for note/volume/etc)
// this is such a common thing, I made a helper for it
void gen_midi_float(char* name, NullUnitParamInfo* out) {
  NullUnitParamValue v;
  out->type = NULL_PARAM_F32;
  v.f = 0.0f;
  out->min = v;
  out->value = v;
  v.f = 127.0f;
  out->max = v;
  out->name = strdup(name);
}

// initialize a u32 0-127 (for note/volume/etc)
// this is such a common thing, I made a helper for it
void gen_midi_unsigned(char* name, NullUnitParamInfo* out) {
  NullUnitParamValue v;
  out->type = NULL_PARAM_U32;
  v.u = 0;
  out->min = v;
  out->value = v;
  v.u = 127;
  out->max = v;
  out->name = strdup(name);
}
