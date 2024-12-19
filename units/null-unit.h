// use this header in all C null-units

#include <stdint.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 48000.0f
#endif

// these are exposed from host

// get some named bytes (sample, etc) from host
__attribute__((import_module("env"), import_name("get_data_floats")))
void get_data_floats(unsigned int id, unsigned int offset, unsigned int length, float* out);


// these are exposed from a unit

typedef enum {
  NULL_PARAM_BOOL,
  NULL_PARAM_U8,
  NULL_PARAM_I8,
  NULL_PARAM_U16,
  NULL_PARAM_I16,
  NULL_PARAM_U32,
  NULL_PARAM_I32,
  NULL_PARAM_F32,
  NULL_PARAM_U64,
  NULL_PARAM_I64,
  NULL_PARAM_F64
}  NullUnitParamType;

typedef union {
  bool boolean;
  int8_t i8;
  int16_t i16;
  int32_t i32;
  int64_t i64;
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  float f32;
  double f64;
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

// Get info about the unit
__attribute__((export_name("get_info")))
NullUnitnInfo* get_info();

// process a single value, in a 0-255 position frame, return output
__attribute__((export_name("process")))
float process(uint8_t position, float input, uint8_t channel);

// called when you plugin is unloaded
__attribute__((export_name("destroy")))
void destroy();

// set a parameter
__attribute__((export_name("param_set")))
void param_set(uint8_t paramId, NullUnitParamValue* value);

// get the current value of a parameter
__attribute__((export_name("param_get")))
NullUnitParamValue* param_get(uint8_t paramId);

// HELPERS
// these may not be available in other languages or work the same

// initialize a f32 0-127 (for note/volume/etc)
void midi_float(char* name, NullUnitParamInfo* out) {
  NullUnitParamValue v;
  out->type = NULL_PARAM_F32;
  v.f32 = 0.0f;
  out->min = v;
  v.f32 = 127.0f;
  out->max = v;
  v.f32 = 0.0f;
  out->value = v;
  out->name = strdup(name);
}

// get the string representation of a single param
void param_string(NullUnitParamInfo param, char* out) {
  switch(param.type) {
    case NULL_PARAM_BOOL: snprintf(out, 100, "%s: %s", param.name, param.value.boolean ? "true" : "false"); break;
    case NULL_PARAM_U8: snprintf(out, 100, "%s: %u", param.name, param.value.u8); break;
    case NULL_PARAM_I8: snprintf(out, 100, "%s: %d", param.name, param.value.i8); break;
    case NULL_PARAM_U16: snprintf(out, 100, "%s: %u", param.name, param.value.u16); break;
    case NULL_PARAM_I16: snprintf(out, 100, "%s: %d", param.name, param.value.i16); break;
    case NULL_PARAM_U32: snprintf(out, 100, "%s: %u", param.name, param.value.u32); break;
    case NULL_PARAM_I32: snprintf(out, 100, "%s: %d", param.name, param.value.i32); break;
    case NULL_PARAM_F32: snprintf(out, 100, "%s: %f", param.name, param.value.f32); break;
    case NULL_PARAM_U64: snprintf(out, 100, "%s: %llu", param.name, param.value.u64); break;
    case NULL_PARAM_I64: snprintf(out, 100, "%s: %lld", param.name, param.value.i64); break;
    case NULL_PARAM_F64: snprintf(out, 100, "%s: %f", param.name, param.value.f64); break;
    default: snprintf(out, 100, "%s: unkown", param.name); break;
  }
}

// debug function to show my current params
void show_info() {
  // disable printf buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  NullUnitnInfo* info = get_info();
  printf("%s\nin: %u\nout: %u\nparams: %u\n", info->name, info->channelsIn, info->channelsOut, info->paramCount);
  char p[100];
  for (int i=0;i< info->paramCount;i++) {
    param_string(info->params[i], p);
    printf("  %s\n", p);
  }
}

// midi note num to frequency
float noteToFreq(int note) {
    float a = 440; //frequency of A (coomon value is 440Hz)
    return (a / 32) * pow(2, ((note - 9) / 12.0));
}
