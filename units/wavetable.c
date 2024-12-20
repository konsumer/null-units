// simple looping sampler null-unit
// that can be used as a wave-table oscillator
// eventually, I might embed wav-tables
// but I am testing loading sample-data from host

#include "null-unit.h"

NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 2

#define PARAM_TYPE 0
#define PARAM_NOTE 1

#define SAMPLE_COUNT 256
#define FRAME_SIZE 256.0f

float sample[SAMPLE_COUNT] = {};

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

  gen_midi_unsigned("type", &unitInfo.params[PARAM_TYPE]); // u32 0-3 wave-type: sin/sqr/tri/saw
  gen_midi_float("note", &unitInfo.params[PARAM_NOTE]); // f32 0-127 midi frequency
  unitInfo.params[PARAM_TYPE].max.u = 3; // (0-3)

  // setup initial sample from host
  get_data_floats(0, 0, SAMPLE_COUNT, sample);

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    float freq = noteToFreq(unitInfo.params[PARAM_NOTE].value.f);

    // Calculate how many cycles should occur in one frame
    float cyclesPerFrame = (freq * FRAME_SIZE) / sampleRate;

    float scaledPos = (position * cyclesPerFrame);

    unsigned int samplePos = (unsigned int)scaledPos % SAMPLE_COUNT;
    return sample[samplePos];
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

  // change sample
  if (paramId == PARAM_TYPE && value->u != unitInfo.params[PARAM_TYPE].value.u) {
    get_data_floats(value->u, 0, SAMPLE_COUNT, sample);
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
