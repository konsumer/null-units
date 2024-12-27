// this is an exmaple unit that shows composite synthesis.
// it emulates the full sound-engine of the Roland 808 drum machine

#include "null-unit.h"
#include <stdlib.h>
#include <math.h>

static NullUnitnInfo unitInfo;

// Parameter indices
#define PARAM_COUNT 9
#define PARAM_NOTE     0
#define PARAM_VOLUME   1
#define PARAM_BD_TONE  2
#define PARAM_BD_DECAY 3
#define PARAM_SD_TONE  4
#define PARAM_SD_SNAPPY 5
#define PARAM_CY_TONE  6
#define PARAM_CY_DECAY 7
#define PARAM_OH_DECAY 8

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "tr808",
        .channelsIn = 0,  // No input needed
        .channelsOut = 2, // Stereo output
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Setup all parameters
    gen_midi_float("note", &unitInfo.params[PARAM_NOTE]);
    gen_midi_float("volume", &unitInfo.params[PARAM_VOLUME]);
    gen_midi_float("BD tone", &unitInfo.params[PARAM_BD_TONE]);
    gen_midi_float("BD decay", &unitInfo.params[PARAM_BD_DECAY]);
    gen_midi_float("SD tone", &unitInfo.params[PARAM_SD_TONE]);
    gen_midi_float("SD snappy", &unitInfo.params[PARAM_SD_SNAPPY]);
    gen_midi_float("CY tone", &unitInfo.params[PARAM_CY_TONE]);
    gen_midi_float("CY decay", &unitInfo.params[PARAM_CY_DECAY]);
    gen_midi_float("OH decay", &unitInfo.params[PARAM_OH_DECAY]);

    return 0;
}

void destroy() {
    for (int i = 0; i < PARAM_COUNT; i++) {
        free(unitInfo.params[i].name);
    }
    free(unitInfo.params);
}

// State variables for each voice
typedef struct {
    float phase;         // Oscillator phase
    float envelope;      // Amplitude envelope
    float pitch_env;     // Pitch envelope
    float noise_env;     // Noise envelope
    float trigger;       // Trigger state
    float last_output;   // For filters
    float noise_state;   // Noise generator state
} DrumVoice;

static DrumVoice voices[16]; // One for each possible drum sound

// Utility functions
static float fast_sin(float x) {
    x = fmod(x, 2.0f * M_PI);
    return sinf(x);
}

static float noise() {
    return (float)rand() / RAND_MAX * 2.0f - 1.0f;
}

static float envelope(float* env, float attack, float decay, float sampleRate) {
    if (*env > 0.0f) {
        *env *= expf(-1.0f / (decay * sampleRate));
    }
    return *env;
}

// Bass Drum
static float synthesize_bd(DrumVoice* voice, float tone, float decay, float sampleRate) {
    float freq = 50.0f + tone * 0.5f;
    float decay_time = 0.1f + decay * 0.01f;

    // Trigger logic
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->pitch_env = 1.0f;
        voice->trigger = 0.0f;
    }

    // Pitch envelope
    voice->pitch_env *= expf(-1.0f / (0.01f * sampleRate));
    float pitch = freq * (1.0f + voice->pitch_env * 2.0f);

    // Oscillator
    voice->phase += 2.0f * M_PI * pitch / sampleRate;
    float osc = fast_sin(voice->phase);

    // Amplitude envelope
    voice->envelope *= expf(-1.0f / (decay_time * sampleRate));

    return osc * voice->envelope;
}

// Snare Drum
static float synthesize_sd(DrumVoice* voice, float tone, float snappy, float sampleRate) {
    float freq = 200.0f + tone * 2.0f;
    float noise_amount = snappy / 127.0f;

    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->noise_env = 1.0f;
        voice->trigger = 0.0f;
    }

    // Oscillator
    voice->phase += 2.0f * M_PI * freq / sampleRate;
    float osc = fast_sin(voice->phase);

    // Noise component
    float noise_sig = noise();
    voice->noise_env *= expf(-1.0f / (0.1f * sampleRate));

    // Mix oscillator and noise
    voice->envelope *= expf(-1.0f / (0.1f * sampleRate));
    return (osc * (1.0f - noise_amount) + noise_sig * noise_amount * voice->noise_env) * voice->envelope;
}

// Hi-Hat (Closed)
static float synthesize_ch(DrumVoice* voice, float tone, float sampleRate) {
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->trigger = 0.0f;
    }

    // Six square waves at different frequencies
    float freqs[6] = {2000.0f, 3000.0f, 4500.0f, 6000.0f, 8000.0f, 10000.0f};
    float mix = 0.0f;

    for (int i = 0; i < 6; i++) {
        voice->phase += 2.0f * M_PI * freqs[i] / sampleRate;
        mix += (fast_sin(voice->phase) > 0.0f) ? 1.0f : -1.0f;
    }

    // Short decay
    voice->envelope *= expf(-1.0f / (0.05f * sampleRate));
    return (mix / 6.0f) * voice->envelope;
}

// Open Hi-Hat
static float synthesize_oh(DrumVoice* voice, float decay, float sampleRate) {
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->trigger = 0.0f;
    }

    float decay_time = 0.1f + decay * 0.01f;
    float noise_sig = noise();

    // Longer decay than closed hi-hat
    voice->envelope *= expf(-1.0f / (decay_time * sampleRate));
    return noise_sig * voice->envelope;
}

// Cymbal
static float synthesize_cy(DrumVoice* voice, float tone, float decay, float sampleRate) {
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->trigger = 0.0f;
    }

    float decay_time = 0.2f + decay * 0.02f;
    float noise_sig = noise();

    // High-pass filter simulation
    float filtered = noise_sig - voice->last_output;
    voice->last_output = noise_sig;

    voice->envelope *= expf(-1.0f / (decay_time * sampleRate));
    return filtered * voice->envelope;
}

// Claves
static float synthesize_cl(DrumVoice* voice, float sampleRate) {
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->trigger = 0.0f;
    }

    float freq = 2500.0f;
    voice->phase += 2.0f * M_PI * freq / sampleRate;
    float osc = fast_sin(voice->phase);

    // Very short decay
    voice->envelope *= expf(-1.0f / (0.02f * sampleRate));
    return osc * voice->envelope;
}

// Cowbell
static float synthesize_cb(DrumVoice* voice, float sampleRate) {
    if (voice->trigger > 0.0f) {
        voice->envelope = 1.0f;
        voice->trigger = 0.0f;
    }

    // Two square waves
    float freq1 = 540.0f;
    float freq2 = 800.0f;

    float osc1 = fast_sin(voice->phase);
    float osc2 = fast_sin(voice->phase * freq2/freq1);

    voice->phase += 2.0f * M_PI * freq1 / sampleRate;

    voice->envelope *= expf(-1.0f / (0.1f * sampleRate));
    return (osc1 + osc2) * 0.5f * voice->envelope;
}

// TODO: detect note-chnage better (use set_param)
static float last_sample_note = -1.0f;

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    float output = 0.0f;
    float note = unitInfo.params[PARAM_NOTE].value.f;
    float volume = unitInfo.params[PARAM_VOLUME].value.f / 127.0f;

    // Trigger on note changes
    if (note != last_sample_note) {
        // If we see a new note, or if we see the same note after seeing a different note
        if (note > 0 && (last_sample_note <= 0 || (int)note != (int)last_sample_note)) {
            switch ((int)note) {
                case 35:  // BD
                    voices[0].trigger = 1.0f;
                    break;
                case 38:  // SD
                    voices[1].trigger = 1.0f;
                    break;
                case 42:  // CH
                    voices[2].trigger = 1.0f;
                    break;
                case 51:  // OH
                    voices[3].trigger = 1.0f;
                    break;
                case 49:  // CY
                    voices[4].trigger = 1.0f;
                    break;
                case 39:  // CL
                    voices[5].trigger = 1.0f;
                    break;
                case 46:  // CB
                    voices[6].trigger = 1.0f;
                    break;
            }
        }
        last_sample_note = note;
    }

    // Process all voices
    output += synthesize_bd(&voices[0],
                          unitInfo.params[PARAM_BD_TONE].value.f,
                          unitInfo.params[PARAM_BD_DECAY].value.f,
                          sampleRate);

    output += synthesize_sd(&voices[1],
                          unitInfo.params[PARAM_SD_TONE].value.f,
                          unitInfo.params[PARAM_SD_SNAPPY].value.f,
                          sampleRate);

    output += synthesize_ch(&voices[2], 0.0f, sampleRate);
    output += synthesize_oh(&voices[3],
                          unitInfo.params[PARAM_OH_DECAY].value.f,
                          sampleRate);
    output += synthesize_cy(&voices[4],
                          unitInfo.params[PARAM_CY_TONE].value.f,
                          unitInfo.params[PARAM_CY_DECAY].value.f,
                          sampleRate);
    output += synthesize_cl(&voices[5], sampleRate);
    output += synthesize_cb(&voices[6], sampleRate);

    output *= 0.25f * volume;

    if (output > 1.0f) output = 1.0f;
    if (output < -1.0f) output = -1.0f;

    return output;
}

NullUnitnInfo* get_info() {
    return &unitInfo;
}

void param_set(uint8_t paramId, NullUnitParamValue* value) {
    if (paramId >= PARAM_COUNT) return;
    unitInfo.params[paramId].value = *value;
}

NullUnitParamValue* param_get(uint8_t paramId) {
    if (paramId >= PARAM_COUNT) return NULL;
    return &unitInfo.params[paramId].value;
}
