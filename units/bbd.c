// "bucket brigade" style delay, that mimics devices from 60s/70s

#include "null-unit.h"
#include <math.h>
#include <stdlib.h>

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 3
#define PARAM_TIME 0    // Delay time (in ms)
#define PARAM_FEEDBACK 1 // Feedback amount (0-127)
#define PARAM_DIRT 2    // Amount of "dirt" (0-127) - controls filtering and noise

typedef struct {
    float* buffer;
    size_t buffer_size;
    size_t write_pos;
    float last_output;
    uint32_t noise_seed;
} DelayState;

static DelayState state;

// Simple white noise generator
static float white_noise(uint32_t* seed) {
    *seed = *seed * 1664525 + 1013904223;
    return ((float)(*seed) / (float)UINT32_MAX) * 2.0f - 1.0f;
}

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "bbd",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Time in milliseconds
    unitInfo.params[0] = (NullUnitParamInfo) {
        .name = "time",
        .value = {.f = 0.0f},
        .min = {.f = 1.0f},
        .max = {.f = 4000.0f},
        .type = NULL_PARAM_F32
    };

    // Feedback amount (MIDI range)
    gen_midi_float("feedback", &unitInfo.params[PARAM_FEEDBACK]);

    // Dirt amount (MIDI range)
    gen_midi_float("dirt", &unitInfo.params[PARAM_DIRT]);

    // Initialize state
    state.buffer_size = (unitInfo.params[0].max.f / 1000.0f) * 48000; // Assuming max 48kHz
    state.buffer = calloc(state.buffer_size, sizeof(float));
    state.write_pos = 0;
    state.last_output = 0.0f;
    state.noise_seed = 12345;

    return 0;
}

void destroy() {
    if (state.buffer) {
        free(state.buffer);
        state.buffer = NULL;
    }
}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    // Parameter processing
    float delay_ms = unitInfo.params[PARAM_TIME].value.f;
    float feedback = unitInfo.params[PARAM_FEEDBACK].value.f / 127.0f; // Normalize to 0-1
    float dirt = unitInfo.params[PARAM_DIRT].value.f / 127.0f; // Normalize to 0-1

    // Calculate delay in samples
    float delay_samples = (delay_ms / 1000.0f) * sampleRate;
    delay_samples = fmaxf(fminf(delay_samples, state.buffer_size - 1), 1.0f);

    // Calculate read position
    float read_pos = state.write_pos - delay_samples;
    if (read_pos < 0) {
        read_pos += state.buffer_size;
    }

    // Linear interpolation for fractional delays
    size_t read_pos_floor = (size_t)read_pos;
    size_t read_pos_ceil = (read_pos_floor + 1) % state.buffer_size;
    float frac = read_pos - (float)read_pos_floor;

    float sample1 = state.buffer[read_pos_floor];
    float sample2 = state.buffer[read_pos_ceil];
    float delayed_signal = sample1 + frac * (sample2 - sample1);

    // Add some noise and filtering based on dirt parameter
    float noise = white_noise(&state.noise_seed) * 0.01f * dirt;
    delayed_signal = delayed_signal * (1.0f - dirt * 0.3f) + noise;

    // Simple lowpass filtering
    delayed_signal = 0.7f * delayed_signal + 0.3f * state.last_output;

    // Write to buffer
    float write_sample = input + delayed_signal * feedback;
    // Soft clipping
    write_sample = tanhf(write_sample);

    state.buffer[state.write_pos] = write_sample;
    state.write_pos = (state.write_pos + 1) % state.buffer_size;
    state.last_output = delayed_signal;

    return delayed_signal;
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
