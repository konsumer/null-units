// simple example with only 1 param: gain (0-127 float, 127 is 2X)
// think of this as a template

#include "null-unit.h"

static NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 1

#define PARAM_GAIN 0

// called when the unit is loaded, returns the number of params it accepts
int main(int argc, char *argv[]) {
  NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo) {
    .name= "gain",
    .channelsIn = 1,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  gen_midi_float("gain", &unitInfo.params[PARAM_GAIN]);

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(uint8_t position, float input, uint8_t channel, float sampleRate) {
  float gain = unitInfo.params[PARAM_GAIN].value.f / 64.0f;
  return input * gain;
}

// Get info about the unit
NullUnitnInfo* get_info() {
  return &unitInfo;
}

// set a parameter
void param_set(uint8_t paramId, NullUnitParamValue* value) {
  if (paramId>= PARAM_COUNT) {
    return;
  }
  unitInfo.params[paramId].value = *value;
  // show_info();
}

// get the current value of a parameter
NullUnitParamValue* param_get(uint8_t paramId) {
  if (paramId>= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}
