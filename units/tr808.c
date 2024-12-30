#include "null-unit.h"

static NullUnitnInfo unitInfo;

typedef enum {
  PARAM_NOTE,

  // Bass Drum (BD) parameters
  PARAM_BD_AMP_DECAY,   // ms
  PARAM_BD_BASE_FREQ,   // midi note
  PARAM_BD_DISTORTION,  // 0-127
  PARAM_BD_NOISE_MIX,   // 0-127
  PARAM_BD_PITCH_DECAY, // ms
  PARAM_BD_PITCH_MOD,   // 0-127

  // Snare Drum (SD) parameters
  PARAM_SD_AMP_DECAY,   // ms
  PARAM_SD_BASE_FREQ,   // midi note
  PARAM_SD_DISTORTION,  // 0-127
  PARAM_SD_NOISE_MIX,   // 0-127
  PARAM_SD_PITCH_DECAY, // ms
  PARAM_SD_PITCH_MOD,   // 0-127
  PARAM_SD_FILTER_FREQ, // midi note
  PARAM_SD_FILTER_Q,    // 0-127

  // Closed Hat (CH) parameters
  PARAM_CH_AMP_DECAY,   // ms
  PARAM_CH_BASE_FREQ,   // midi note
  PARAM_CH_FILTER_FREQ, // midi note
  PARAM_CH_FILTER_Q,    // 0-127

  // Open Hat (OH) parameters
  PARAM_OH_AMP_DECAY,   // ms
  PARAM_OH_BASE_FREQ,   // midi note
  PARAM_OH_FILTER_FREQ, // midi note
  PARAM_OH_FILTER_Q,    // 0-127

  // Clap (CL) parameters
  PARAM_CL_AMP_DECAY,   // ms
  PARAM_CL_BASE_FREQ,   // midi note
  PARAM_CL_DISTORTION,  // 0-127
  PARAM_CL_FILTER_FREQ, // midi note
  PARAM_CL_FILTER_Q,    // 0-127

  // Cowbell (CP) parameters
  PARAM_CP_AMP_DECAY,   // ms
  PARAM_CP_BASE_FREQ,   // midi note
  PARAM_CP_DISTORTION,  // 0-127
  PARAM_CP_FILTER_FREQ, // midi note
  PARAM_CP_FILTER_Q,    // 0-127
  PARAM_CP_PITCH_MOD,   // 0-127

  // Rim Shot (RS) parameters
  PARAM_RS_AMP_DECAY,   // ms
  PARAM_RS_BASE_FREQ,   // midi note
  PARAM_RS_DISTORTION,  // 0-127
  PARAM_RS_FILTER_FREQ, // midi note
  PARAM_RS_FILTER_Q,    // 0-127
  PARAM_RS_NOISE_MIX,   // 0-127
  PARAM_RS_PITCH_MOD,   // 0-127

  // Cymbal (CY) parameters
  PARAM_CY_AMP_DECAY,   // ms
  PARAM_CY_BASE_FREQ,   // midi note
  PARAM_CY_DISTORTION,  // 0-127
  PARAM_CY_FILTER_FREQ, // midi note
  PARAM_CY_FILTER_Q,    // 0-127

  PARAM_COUNT
} NullUnit808Param;

typedef enum { OSC_SINE, OSC_TRIANGLE, OSC_SAWTOOTH, OSC_NOISE } OscType;

typedef enum {
  VOICE_BD,
  VOICE_SD,
  VOICE_CH,
  VOICE_OH,
  VOICE_CL,
  VOICE_CP,
  VOICE_RS,
  VOICE_CY,

  VOICE_COUNT
} VoiceType;

typedef struct {
  float pitchEnv;
  float ampEnv;
  float phase;
  float velocity;
  bool triggered;
  float filterState;
  OscType oscType;
  bool useFilter;
} VoiceState;

static VoiceState voices[VOICE_COUNT] = {};

static float filter(float input, float *state, float freq, float sampleRate) {
  float alpha = freq / (freq + sampleRate);
  *state = *state + alpha * (input - *state);
  return *state;
}

int main(int argc, char *argv[]) {
  NullUnitParamInfo *params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

  unitInfo = (NullUnitnInfo){ .name = "tr808", .channelsIn = 0, .channelsOut = 1, .paramCount = PARAM_COUNT, .params = params };

  // Note parameter (0-127: midi note)
  gen_midi_int("note", &unitInfo.params[PARAM_NOTE]);

  // Bass Drum parameters (0.0-127.0)
  gen_midi_float("BD amp decay", &unitInfo.params[PARAM_BD_AMP_DECAY]);
  gen_midi_float("BD base freq", &unitInfo.params[PARAM_BD_BASE_FREQ]);
  gen_midi_float("BD distortion", &unitInfo.params[PARAM_BD_DISTORTION]);
  gen_midi_float("BD noise mix", &unitInfo.params[PARAM_BD_NOISE_MIX]);
  gen_midi_float("BD pitch decay", &unitInfo.params[PARAM_BD_PITCH_DECAY]);
  gen_midi_float("BD pitch mod", &unitInfo.params[PARAM_BD_PITCH_MOD]);

  // Snare Drum parameters
  gen_midi_float("SD amp decay", &unitInfo.params[PARAM_SD_AMP_DECAY]);
  gen_midi_float("SD base freq", &unitInfo.params[PARAM_SD_BASE_FREQ]);
  gen_midi_float("SD distortion", &unitInfo.params[PARAM_SD_DISTORTION]);
  gen_midi_float("SD noise mix", &unitInfo.params[PARAM_SD_NOISE_MIX]);
  gen_midi_float("SD pitch decay", &unitInfo.params[PARAM_SD_PITCH_DECAY]);
  gen_midi_float("SD pitch mod", &unitInfo.params[PARAM_SD_PITCH_MOD]);
  gen_midi_float("SD filter freq", &unitInfo.params[PARAM_SD_FILTER_FREQ]);
  gen_midi_float("SD filter Q", &unitInfo.params[PARAM_SD_FILTER_Q]);

  // Closed Hat parameters
  gen_midi_float("CH amp decay", &unitInfo.params[PARAM_CH_AMP_DECAY]);
  gen_midi_float("CH base freq", &unitInfo.params[PARAM_CH_BASE_FREQ]);
  gen_midi_float("CH filter freq", &unitInfo.params[PARAM_CH_FILTER_FREQ]);
  gen_midi_float("CH filter Q", &unitInfo.params[PARAM_CH_FILTER_Q]);

  // Open Hat parameters
  gen_midi_float("OH amp decay", &unitInfo.params[PARAM_OH_AMP_DECAY]);
  gen_midi_float("OH base freq", &unitInfo.params[PARAM_OH_BASE_FREQ]);
  gen_midi_float("OH filter freq", &unitInfo.params[PARAM_OH_FILTER_FREQ]);
  gen_midi_float("OH filter Q", &unitInfo.params[PARAM_OH_FILTER_Q]);

  // Clap parameters
  gen_midi_float("CL amp decay", &unitInfo.params[PARAM_CL_AMP_DECAY]);
  gen_midi_float("CL base freq", &unitInfo.params[PARAM_CL_BASE_FREQ]);
  gen_midi_float("CL distortion", &unitInfo.params[PARAM_CL_DISTORTION]);
  gen_midi_float("CL filter freq", &unitInfo.params[PARAM_CL_FILTER_FREQ]);
  gen_midi_float("CL filter Q", &unitInfo.params[PARAM_CL_FILTER_Q]);

  // Cowbell parameters
  gen_midi_float("CP amp decay", &unitInfo.params[PARAM_CP_AMP_DECAY]);
  gen_midi_float("CP base freq", &unitInfo.params[PARAM_CP_BASE_FREQ]);
  gen_midi_float("CP distortion", &unitInfo.params[PARAM_CP_DISTORTION]);
  gen_midi_float("CP filter freq", &unitInfo.params[PARAM_CP_FILTER_FREQ]);
  gen_midi_float("CP filter Q", &unitInfo.params[PARAM_CP_FILTER_Q]);
  gen_midi_float("CP pitch mod", &unitInfo.params[PARAM_CP_PITCH_MOD]);

  // Rim Shot parameters
  gen_midi_float("RS amp decay", &unitInfo.params[PARAM_RS_AMP_DECAY]);
  gen_midi_float("RS base freq", &unitInfo.params[PARAM_RS_BASE_FREQ]);
  gen_midi_float("RS distortion", &unitInfo.params[PARAM_RS_DISTORTION]);
  gen_midi_float("RS filter freq", &unitInfo.params[PARAM_RS_FILTER_FREQ]);
  gen_midi_float("RS filter Q", &unitInfo.params[PARAM_RS_FILTER_Q]);
  gen_midi_float("RS noise mix", &unitInfo.params[PARAM_RS_NOISE_MIX]);
  gen_midi_float("RS pitch mod", &unitInfo.params[PARAM_RS_PITCH_MOD]);

  // Cymbal parameters
  gen_midi_float("CY amp decay", &unitInfo.params[PARAM_CY_AMP_DECAY]);
  gen_midi_float("CY base freq", &unitInfo.params[PARAM_CY_BASE_FREQ]);
  gen_midi_float("CY distortion", &unitInfo.params[PARAM_CY_DISTORTION]);
  gen_midi_float("CY filter freq", &unitInfo.params[PARAM_CY_FILTER_FREQ]);
  gen_midi_float("CY filter Q", &unitInfo.params[PARAM_CY_FILTER_Q]);

  // Initialize BD default values (matching your original values)
  unitInfo.params[PARAM_BD_AMP_DECAY].value.f = 100.0f; // 0.1s * 1000
  unitInfo.params[PARAM_BD_BASE_FREQ].value.f = freqToNote(55.0f);
  unitInfo.params[PARAM_BD_DISTORTION].value.f = 25.4f; // 0.2 * 127
  unitInfo.params[PARAM_BD_NOISE_MIX].value.f = 0.0f;
  unitInfo.params[PARAM_BD_PITCH_DECAY].value.f = 50.0f; // 0.05s * 1000
  unitInfo.params[PARAM_BD_PITCH_MOD].value.f = 100.0f;

  // Initialize SD default values
  unitInfo.params[PARAM_SD_AMP_DECAY].value.f = 80.0f; // 0.08s * 1000
  unitInfo.params[PARAM_SD_BASE_FREQ].value.f = freqToNote(220.0f);
  unitInfo.params[PARAM_SD_DISTORTION].value.f = 12.7f;  // 0.1 * 127
  unitInfo.params[PARAM_SD_NOISE_MIX].value.f = 63.5f;   // 0.5 * 127
  unitInfo.params[PARAM_SD_PITCH_DECAY].value.f = 20.0f; // 0.02s * 1000
  unitInfo.params[PARAM_SD_PITCH_MOD].value.f = 50.0f;
  unitInfo.params[PARAM_SD_FILTER_FREQ].value.f = freqToNote(2000.0f);
  unitInfo.params[PARAM_SD_FILTER_Q].value.f = 127.0f;

  // Initialize CH default values
  unitInfo.params[PARAM_CH_AMP_DECAY].value.f = 30.0f; // 0.03s * 1000
  unitInfo.params[PARAM_CH_BASE_FREQ].value.f = freqToNote(2500.0f);
  unitInfo.params[PARAM_CH_FILTER_FREQ].value.f = freqToNote(8000.0f);
  unitInfo.params[PARAM_CH_FILTER_Q].value.f = 254.0f; // 2.0 * 127

  // Initialize OH default values
  unitInfo.params[PARAM_OH_AMP_DECAY].value.f = 300.0f; // 0.3s * 1000
  unitInfo.params[PARAM_OH_BASE_FREQ].value.f = freqToNote(2500.0f);
  unitInfo.params[PARAM_OH_FILTER_FREQ].value.f = freqToNote(8000.0f);
  unitInfo.params[PARAM_OH_FILTER_Q].value.f = 254.0f; // 2.0 * 127

  // Initialize CL default values
  unitInfo.params[PARAM_CL_AMP_DECAY].value.f = 150.0f; // 0.15s * 1000
  unitInfo.params[PARAM_CL_BASE_FREQ].value.f = freqToNote(1500.0f);
  unitInfo.params[PARAM_CL_DISTORTION].value.f = 12.7f; // 0.1 * 127
  unitInfo.params[PARAM_CL_FILTER_FREQ].value.f = freqToNote(1000.0f);
  unitInfo.params[PARAM_CL_FILTER_Q].value.f = 190.5f; // 1.5 * 127

  // Initialize CP default values
  unitInfo.params[PARAM_CP_AMP_DECAY].value.f = 150.0f; // 0.15s * 1000
  unitInfo.params[PARAM_CP_BASE_FREQ].value.f = freqToNote(800.0f);
  unitInfo.params[PARAM_CP_DISTORTION].value.f = 12.7f; // 0.1 * 127
  unitInfo.params[PARAM_CP_FILTER_FREQ].value.f = freqToNote(2000.0f);
  unitInfo.params[PARAM_CP_FILTER_Q].value.f = 127.0f; // 1.0 * 127
  unitInfo.params[PARAM_CP_PITCH_MOD].value.f = 20.0f;

  // Initialize RS default values
  unitInfo.params[PARAM_RS_AMP_DECAY].value.f = 40.0f; // 0.04s * 1000
  unitInfo.params[PARAM_RS_BASE_FREQ].value.f = freqToNote(1700.0f);
  unitInfo.params[PARAM_RS_DISTORTION].value.f = 38.1f; // 0.3 * 127
  unitInfo.params[PARAM_RS_FILTER_FREQ].value.f = freqToNote(4000.0f);
  unitInfo.params[PARAM_RS_FILTER_Q].value.f = 127.0f; // 1.0 * 127
  unitInfo.params[PARAM_RS_NOISE_MIX].value.f = 38.1f; // 0.3 * 127
  unitInfo.params[PARAM_RS_PITCH_MOD].value.f = 10.0f;

  // Initialize CY default values
  unitInfo.params[PARAM_CY_AMP_DECAY].value.f = 500.0f; // 0.5s * 1000
  unitInfo.params[PARAM_CY_BASE_FREQ].value.f = freqToNote(3000.0f);
  unitInfo.params[PARAM_CY_DISTORTION].value.f = 12.7f; // 0.1 * 127
  unitInfo.params[PARAM_CY_FILTER_FREQ].value.f = freqToNote(5000.0f);
  unitInfo.params[PARAM_CY_FILTER_Q].value.f = 254.0f; // 2.0 * 127

  // Initialize oscillator types and filter flags
  voices[VOICE_BD].oscType = OSC_SINE;
  voices[VOICE_BD].useFilter = false;

  voices[VOICE_SD].oscType = OSC_SINE;
  voices[VOICE_SD].useFilter = true;

  voices[VOICE_CH].oscType = OSC_NOISE;
  voices[VOICE_CH].useFilter = true;

  voices[VOICE_OH].oscType = OSC_NOISE;
  voices[VOICE_OH].useFilter = true;

  voices[VOICE_CL].oscType = OSC_NOISE;
  voices[VOICE_CL].useFilter = true;

  voices[VOICE_CP].oscType = OSC_TRIANGLE;
  voices[VOICE_CP].useFilter = true;

  voices[VOICE_RS].oscType = OSC_SINE;
  voices[VOICE_RS].useFilter = true;

  voices[VOICE_CY].oscType = OSC_NOISE;
  voices[VOICE_CY].useFilter = true;

  return 0;
}

void destroy() {
  if (unitInfo.params) {
    free(unitInfo.params);
  }
}

NullUnitnInfo *get_info() {
  return &unitInfo;
}

float process_voice(VoiceState *voice, VoiceType voiceType, float sampleRate) {
  if (!voice->triggered && voice->ampEnv < 0.001f) {
    return 0.0f;
  }

  // Get parameters based on voice type
  float ampDecay, baseFreq, distortion = 0.0f, noiseMix = 0.0f;
  float pitchDecay = 0.0f, pitchMod = 0.0f, filterFreq = 0.0f, filterQ = 0.0f;

  switch (voiceType) {
  case VOICE_BD:
    ampDecay = unitInfo.params[PARAM_BD_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_BD_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_BD_DISTORTION].value.f / 127.0f;
    noiseMix = unitInfo.params[PARAM_BD_NOISE_MIX].value.f / 127.0f;
    pitchDecay = unitInfo.params[PARAM_BD_PITCH_DECAY].value.f / 1000.0f;
    pitchMod = unitInfo.params[PARAM_BD_PITCH_MOD].value.f;
    break;

  case VOICE_SD:
    ampDecay = unitInfo.params[PARAM_SD_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_SD_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_SD_DISTORTION].value.f / 127.0f;
    noiseMix = unitInfo.params[PARAM_SD_NOISE_MIX].value.f / 127.0f;
    pitchDecay = unitInfo.params[PARAM_SD_PITCH_DECAY].value.f / 1000.0f;
    pitchMod = unitInfo.params[PARAM_SD_PITCH_MOD].value.f;
    filterFreq = noteToFreq(unitInfo.params[PARAM_SD_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_SD_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_CH:
    ampDecay = unitInfo.params[PARAM_CH_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_CH_BASE_FREQ].value.f);
    noiseMix = 1.0f; // Always full noise
    filterFreq = noteToFreq(unitInfo.params[PARAM_CH_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_CH_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_OH:
    ampDecay = unitInfo.params[PARAM_OH_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_OH_BASE_FREQ].value.f);
    noiseMix = 1.0f; // Always full noise
    filterFreq = noteToFreq(unitInfo.params[PARAM_OH_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_OH_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_CL:
    ampDecay = unitInfo.params[PARAM_CL_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_CL_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_CL_DISTORTION].value.f / 127.0f;
    noiseMix = 1.0f; // Always full noise
    filterFreq = noteToFreq(unitInfo.params[PARAM_CL_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_CL_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_CP:
    ampDecay = unitInfo.params[PARAM_CP_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_CP_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_CP_DISTORTION].value.f / 127.0f;
    pitchMod = unitInfo.params[PARAM_CP_PITCH_MOD].value.f;
    filterFreq = noteToFreq(unitInfo.params[PARAM_CP_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_CP_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_RS:
    ampDecay = unitInfo.params[PARAM_RS_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_RS_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_RS_DISTORTION].value.f / 127.0f;
    noiseMix = unitInfo.params[PARAM_RS_NOISE_MIX].value.f / 127.0f;
    pitchMod = unitInfo.params[PARAM_RS_PITCH_MOD].value.f;
    filterFreq = noteToFreq(unitInfo.params[PARAM_RS_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_RS_FILTER_Q].value.f / 127.0f;
    break;

  case VOICE_CY:
    ampDecay = unitInfo.params[PARAM_CY_AMP_DECAY].value.f / 1000.0f;
    baseFreq = noteToFreq(unitInfo.params[PARAM_CY_BASE_FREQ].value.f);
    distortion = unitInfo.params[PARAM_CY_DISTORTION].value.f / 127.0f;
    noiseMix = 1.0f; // Always full noise
    filterFreq = noteToFreq(unitInfo.params[PARAM_CY_FILTER_FREQ].value.f);
    filterQ = unitInfo.params[PARAM_CY_FILTER_Q].value.f / 127.0f;
    break;

  default:
    return 0.0f;
  }

  // Update envelopes
  voice->pitchEnv = nu_envelope(&voice->pitchEnv, 0.0f, pitchDecay, sampleRate);
  voice->ampEnv = nu_envelope(&voice->ampEnv, 0.0f, ampDecay, sampleRate);

  // Calculate frequency with pitch envelope
  float freq = baseFreq + (pitchMod * voice->pitchEnv);

  // Update phase
  voice->phase += freq * 2.0f * M_PI / sampleRate;

  // Generate core sound
  float osc = 0.0f;
  switch (voice->oscType) {
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
  if (noiseMix > 0.0f) {
    osc = osc * (1.0f - noiseMix) + nu_noise() * noiseMix;
  }

  // Apply envelope
  osc *= voice->ampEnv;

  // Apply distortion
  if (distortion > 0.0f) {
    osc *= (1.0f + distortion * osc * osc);
  }

  // Apply filter if needed
  if (voice->useFilter) {
    osc = filter(osc, &voice->filterState, filterFreq, sampleRate);
  }

  voice->triggered = false;
  return osc;
}

void trigger_voice(VoiceType voice, float velocity) {
  if (voice >= VOICE_COUNT) {
    return;
  }
  voices[voice].pitchEnv = 1.0f;
  voices[voice].ampEnv = 1.0f;
  voices[voice].velocity = velocity;
  voices[voice].triggered = true;
}

void param_set(uint8_t paramId, NullUnitParamValue *value) {
  if (paramId >= PARAM_COUNT) {
    return;
  }
  unitInfo.params[paramId].value = *value;

  if (paramId == PARAM_NOTE) {
    switch (value->i) {
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

NullUnitParamValue *param_get(uint8_t paramId) {
  if (paramId >= PARAM_COUNT) {
    return NULL;
  }
  return &unitInfo.params[paramId].value;
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
  float output = 0.0f;

  for (int i = 0; i < VOICE_COUNT; i++) {
    output += process_voice(&voices[i], (VoiceType)i, sampleRate);
  }

  output = fmaxf(-1.0f, fminf(1.0f, output));
  return output;
}
