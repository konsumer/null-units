#pragma once

#define CVECTOR_LOGARITHMIC_GROWTH
#include "cvector.h"

#include <soundio/soundio.h>
#include "wasm_export.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_UNITS 256
#define MAX_SAMPLES 256
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 256

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

typedef struct {
    wasm_module_t* module;
    wasm_module_inst_t* module_inst;
    NullUnitnInfo* info;
    struct SoundIoRingBuffer* input_buffer;
    struct SoundIoRingBuffer* output_buffer;
    bool active;
} NullUnit;

typedef struct {
    struct SoundIo* soundio;
    struct SoundIoDevice* device;
    struct SoundIoOutStream* outstream;
    cvector_vector_type(NullUnit*) units;
    cvector_vector_type(float*) samples;
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
