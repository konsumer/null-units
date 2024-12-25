#ifndef NULL_MANAGER_H
#define NULL_MANAGER_H

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
    NullUnit units[MAX_UNITS];
    unsigned int unit_count;
    float* samples[MAX_SAMPLES];
     unsigned int sample_count;
} NullManager;

// Initialize the audio system and manager
NullManager* null_manager_create(void);

// Clean up
void null_manager_destroy(NullManager* manager);

unsigned int load(NullManager* manager, const char* name);
void unload(NullManager* manager, unsigned int unitId);
void connect(NullManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort);
void disconnect(NullManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort);
void set_param(NullManager* manager, unsigned int unitSourceId, unsigned int paramId, NullUnitParamValue value, unsigned int timefromNowInSeconds);
NullUnitParamValue* get_param(NullManager* manager, unsigned int unitSourceId, unsigned int paramId);
NullUnitnInfo* get_info(NullManager* manager, unsigned int unitSourceId);

#endif
