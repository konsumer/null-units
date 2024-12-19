// simple looping wavetable sampler null-unit
// that can be used as a wave-table oscillator
// eventually, I might embed wav-tables
// but I am testing loading sample-data from host

#include "null-unit.h"

NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 3

// called when the unit is loaded, returns the number of params it accepts
int main(int argc, char *argv[]) {
  NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo) {
    .name= "wavetable",
    .channelsIn = 0,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  midi_float("note", &unitInfo.params[0]);
  midi_float("volume", &unitInfo.params[1]);
  midi_float("type", &unitInfo.params[2]);

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(uint8_t position, float input, uint8_t channel) {
  return input;
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
