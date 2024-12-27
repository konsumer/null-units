// example of a composite-synth that can make drum sounds (similar to TR808)

#include "null-unit.h"

static NullUnitnInfo unitInfo;

// the number of params this can receive
#define PARAM_COUNT 1
#define PARAM_NOTE 0

// Oscillator types
typedef enum {
  OSC_SINE,
  OSC_TRIANGLE,
  OSC_SAWTOOTH,
  OSC_NOISE
} OscType;

// Drum voice types
typedef enum {
  VOICE_BD, // Bass Drum
  VOICE_SD, // Snare Drum
  VOICE_CH, // Closed Hat
  VOICE_OH, // Open Hat
  VOICE_CL, // Clap
  VOICE_CP, // Cowbell
  VOICE_RS, // Rim Shot
  VOICE_CY, // Cymbal
  VOICE_COUNT
} VoiceType;

// Voice parameters
typedef struct {
  float baseFreq;   // Base frequency
  float pitchDecay; // Pitch envelope decay time
  float ampDecay;   // Amplitude decay time
  float pitchMod;   // Pitch modulation amount
  float noiseMix;   // Mix of noise (0 = none, 1 = all noise)
  float distortion; // Distortion amount
  OscType oscType;  // Oscillator type
  bool useFilter;   // Whether to use filter
  float filterFreq; // Filter frequency
  float filterQ;    // Filter resonance
} VoiceParams;

// Voice state
typedef struct {
  float pitchEnv;     // Current pitch envelope value
  float ampEnv;       // Current amplitude envelope value
  float phase;        // Phase for oscillator
  float velocity;     // Current velocity (0.0-1.0)
  bool triggered;     // Whether voice was triggered
  float filterState;  // Simple filter state
  VoiceParams params; // Parameters for this voice
} VoiceState;

// Global state for all voices
static VoiceState voices[VOICE_COUNT] = {};

// Simple one-pole filter
float filter(float input, float *state, float freq, float sampleRate) {
  float alpha = freq / (freq + sampleRate);
  *state = *state + alpha * (input - *state);
  return *state;
}

// Process a single voice
float process_voice(VoiceState *voice, float sampleRate) {
  if (!voice->triggered && voice->ampEnv < 0.001f) {
    return 0.0f;
  }

  // Update envelopes
  voice->pitchEnv = nu_envelope(&voice->pitchEnv, 0.0f, voice->params.pitchDecay, sampleRate);
  voice->ampEnv = nu_envelope(&voice->ampEnv, 0.0f, voice->params.ampDecay, sampleRate);

  // Calculate frequency with pitch envelope
  float freq = voice->params.baseFreq + (voice->params.pitchMod * voice->pitchEnv);

  // Update phase
  voice->phase += freq * 2.0f * M_PI / sampleRate;

  // Generate core sound
  float osc = 0.0f;
  switch (voice->params.oscType) {
  case OSC_SINE:
    osc = nu_sin(voice->phase);
    break;
  case OSC_TRIANGLE:
    osc = nu_triangle(voice->phase);
    break;
  case OSC_SAWTOOTH:
    osc = nu_sawtooth(voice->phase);
    break;
  case OSC_NOISE:
    osc = nu_noise();
    break;
  }

  // Mix with noise if needed
  if (voice->params.noiseMix > 0.0f) {
    osc = osc * (1.0f - voice->params.noiseMix) + nu_noise() * voice->params.noiseMix;
  }

  // Apply envelope
  osc *= voice->ampEnv;

  // Apply distortion
  if (voice->params.distortion > 0.0f) {
    osc *= (1.0f + voice->params.distortion * osc * osc);
  }

  // Apply filter if needed
  if (voice->params.useFilter) {
    osc = filter(osc, &voice->filterState, voice->params.filterFreq, sampleRate);
  }

  voice->triggered = false;
  return osc;
}

// Trigger a voice
void trigger_voice(VoiceType voice, float velocity) {
  if (voice >= VOICE_COUNT) {
    return;
  }
  voices[voice].pitchEnv = 1.0f;
  voices[voice].ampEnv = 1.0f;
  voices[voice].velocity = velocity;
  voices[voice].triggered = true;
}

// called when the unit is loaded, returns the number of params it accepts
int main(int argc, char *argv[]) {
  NullUnitParamInfo *params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo){
    .name = "tr808",
    .channelsIn = 0,
    .channelsOut = 1,
    .paramCount = PARAM_COUNT,
    .params = params
  };

  gen_midi_float("note", &unitInfo.params[PARAM_NOTE]); // f32 0-127 midi frequency

  // TODO: set these up as params with defaults, and scale better (ms, 127, etc)

  voices[VOICE_BD].params = (VoiceParams){
    .ampDecay = 0.1f,
    .baseFreq = 55.0f,
    .distortion = 0.2f,
    .noiseMix = 0.0f,
    .oscType = OSC_SINE,
    .pitchDecay = 0.05f,
    .pitchMod = 100.0f,
    .useFilter = false,
  };

  voices[VOICE_SD].params = (VoiceParams){
    .ampDecay = 0.08f,
    .baseFreq = 220.0f,
    .distortion = 0.1f,
    .filterFreq = 2000.0f,
    .filterQ = 1.0f,
    .noiseMix = 0.5f,
    .oscType = OSC_SINE,
    .pitchDecay = 0.02f,
    .pitchMod = 50.0f,
    .useFilter = true,
  };

  voices[VOICE_CH].params = (VoiceParams){
    .ampDecay = 0.03f,
    .baseFreq = 2500.0f,
    .distortion = 0.0f,
    .filterFreq = 8000.0f,
    .filterQ = 2.0f,
    .noiseMix = 1.0f,
    .oscType = OSC_NOISE,
    .pitchDecay = 0.01f,
    .pitchMod = 0.0f,
    .useFilter = true,
  };

  voices[VOICE_OH].params = (VoiceParams){
    .ampDecay = 0.3f,
    .baseFreq = 2500.0f,
    .distortion = 0.0f,
    .filterFreq = 8000.0f,
    .filterQ = 2.0f,
    .noiseMix = 1.0f,
    .oscType = OSC_NOISE,
    .pitchDecay = 0.01f,
    .pitchMod = 0.0f,
    .useFilter = true,
  };

  voices[VOICE_CL].params = (VoiceParams){
    .ampDecay = 0.15f,
    .baseFreq = 1500.0f,
    .distortion = 0.1f,
    .filterFreq = 1000.0f,
    .filterQ = 1.5f,
    .noiseMix = 1.0f,
    .oscType = OSC_NOISE,
    .pitchDecay = 0.04f,
    .pitchMod = 0.0f,
    .useFilter = true,
  };

  voices[VOICE_CP].params = (VoiceParams){
    .ampDecay = 0.15f,
    .baseFreq = 800.0f,
    .distortion = 0.1f,
    .filterFreq = 2000.0f,
    .filterQ = 1.0f,
    .noiseMix = 0.0f,
    .oscType = OSC_TRIANGLE,
    .pitchDecay = 0.05f,
    .pitchMod = 20.0f,
    .useFilter = true,
  };

  voices[VOICE_RS].params = (VoiceParams){
    .ampDecay = 0.04f,
    .baseFreq = 1700.0f,
    .distortion = 0.3f,
    .filterFreq = 4000.0f,
    .filterQ = 1.0f,
    .noiseMix = 0.3f,
    .oscType = OSC_SINE,
    .pitchDecay = 0.01f,
    .pitchMod = 10.0f,
    .useFilter = true,
  };

  voices[VOICE_CY].params = (VoiceParams){
    .ampDecay = 0.5f,
    .baseFreq = 3000.0f,
    .distortion = 0.1f,
    .filterFreq = 5000.0f,
    .filterQ = 2.0f,
    .noiseMix = 1.0f,
    .oscType = OSC_NOISE,
    .pitchDecay = 0.1f,
    .pitchMod = 0.0f,
    .useFilter = true,
  };

  return 0;
}

// called when you plugin is unloaded
void destroy() {}

// process a single value, in a 0-255 position frame, return output
float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
  float output = 0.0f;

  // Mix all voices
  for (int i = 0; i < VOICE_COUNT; i++) {
    output += process_voice(&voices[i], sampleRate);
  }

  // Prevent clipping
  output = fmaxf(-1.0f, fminf(1.0f, output));

  return output;
}

// Get info about the unit
NullUnitnInfo *get_info() {
  return &unitInfo;
}

// set a parameter
void param_set(uint8_t paramId, NullUnitParamValue *value) {
  if (paramId >= PARAM_COUNT) {
    return;
  }
  unitInfo.params[paramId].value = *value;

  if (paramId == PARAM_NOTE) {
    switch ((int)value->f) {
    case 35: // Acoustic Bass Drum (alternate)
    case 36: // Bass Drum 1
      trigger_voice(VOICE_BD, 1.0f);
      break;

    case 38: // Acoustic Snare
    case 40: // Electric Snare (alternate)
      trigger_voice(VOICE_SD, 1.0f);
      break;

    case 42: // Closed Hi-hat
    case 44: // Pedal Hi-hat (alternate)
      trigger_voice(VOICE_CH, 1.0f);
      break;

    case 46: // Open Hi-hat
      trigger_voice(VOICE_OH, 1.0f);
      break;

    case 39: // Hand Clap
      trigger_voice(VOICE_CL, 1.0f);
      break;

    case 56: // Cowbell
      trigger_voice(VOICE_CP, 1.0f);
      break;

    case 37: // Side Stick/Rim Shot
      trigger_voice(VOICE_RS, 1.0f);
      break;

    case 49: // Crash Cymbal
    case 51: // Ride Cymbal (alternate)
      trigger_voice(VOICE_CY, 1.0f);
      break;
    }
  }
}

// get the current value of a parameter
NullUnitParamValue *param_get(uint8_t paramId) {
  if (paramId >= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}
