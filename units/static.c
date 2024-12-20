// simple example with no params that just outputs white noise

#include "null-unit.h"

static NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 0

// called when the unit is loaded, returns the number of params it accepts
int main(int argc, char *argv[]) {
  NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo) {
    .name= "static",
    .channelsIn = 0,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
  return (((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f;
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
  show_info();
}

// get the current value of a parameter
NullUnitParamValue* param_get(uint8_t paramId) {
  if (paramId>= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}
