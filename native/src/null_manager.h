#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <math.h>

#define CVECTOR_LOGARITHMIC_GROWTH
#include "cvector.h"

#include <soundio/soundio.h>
#include "wasm_export.h"

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 256

// these are the valid types for params
typedef enum {
  NULL_PARAM_BOOL,  // stored as i32
  NULL_PARAM_I32,
  NULL_PARAM_F32
} NullUnitParamType;

// this union is so you can pass around int/float data (for params)
typedef union {
  int32_t i;
  float f;
} NullUnitParamValue;

// this is a single param's info
typedef struct {
  NullUnitParamType type;
  NullUnitParamValue min;
  NullUnitParamValue max;
  NullUnitParamValue value;
  char* name;
} NullUnitParamInfo;

// this is info (name, params, chnnels, etc) about a unit
typedef struct {
  char* name;
  uint8_t channelsIn;
  uint8_t channelsOut;
  cvector_vector_type(NullUnitParamInfo*) params;
} NullUnitnInfo;

// this is a single loaded unit
typedef struct {
    wasm_module_t* module;
    wasm_module_inst_t* module_inst;
    NullUnitnInfo* info;
    struct SoundIoRingBuffer* input_buffer;
    struct SoundIoRingBuffer* output_buffer;
    bool active;
} NullUnit;

// this is info about an available unit
typedef struct {
  char* name;
  char* path;
} NullUnitAvailable;

// this is a loaded sample
typedef struct {
  float* data;
  int len;
} NullUnitSample;

// this represents a complete manager instance
typedef struct {
    struct SoundIo* soundio;
    struct SoundIoDevice* device;
    struct SoundIoOutStream* outstream;
    cvector_vector_type(NullUnitSample) samples;
    cvector_vector_type(NullUnit*) units; // these are loaded
    cvector_vector_type(NullUnitAvailable) available_units; // these are found via paths or whatever
} NullUnitManager;

// Initialize the audio system and manager
NullUnitManager* null_manager_create(void);

// Clean up
void null_manager_destroy(NullUnitManager* manager);

// load a unit
unsigned int null_manager_load(NullUnitManager* manager, const char* name);

// unload a unit
void null_manager_unload(NullUnitManager* manager, unsigned int unitId);

// connect a unit to another
void null_manager_connect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort);

// disconnect
void null_manager_disconnect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort);

// set a param of a unit
void null_manager_set_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId, NullUnitParamValue value, float timefromNowInSeconds);

// get a param of a unit
NullUnitParamValue* null_manager_get_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId);

// get info about a loaded unit
NullUnitnInfo* null_manager_get_info(NullUnitManager* manager, unsigned int unitSourceId);


// load list of wasm files in a dir into manager->available_units
void null_manager_get_units(NullUnitManager* manager, const char* dirname);

// just read a file as bytes
unsigned char* null_manager_read_file(char* filename, int* bytesRead);

// run on every audio-frame (put in your update-loop)
void null_manager_process();
