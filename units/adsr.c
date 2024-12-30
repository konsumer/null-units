// ADSR envelope generator

#include "null-unit.h"

static NullUnitnInfo unitInfo;

#define PARAM_COUNT 5
#define PARAM_TRIGGER 0
#define PARAM_ATTACK 1
#define PARAM_DECAY 2
#define PARAM_SUSTAIN 3
#define PARAM_RELEASE 4

// ADSR state
static struct {
    enum {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    } state;
    float currentLevel;
    float releaseLevel;
    bool noteOn;
    double noteOnTime;
    double noteOffTime;
} envelope = {
    .state = IDLE,
    .currentLevel = 0.0f,
    .releaseLevel = 0.0f,
    .noteOn = false,
    .noteOnTime = 0.0,
    .noteOffTime = 0.0
};

int main(int argc, char *argv[]) {
    NullUnitParamInfo* params = malloc(PARAM_COUNT * sizeof(NullUnitParamInfo));

    unitInfo = (NullUnitnInfo) {
        .name = "adsr",
        .channelsIn = 1,
        .channelsOut = 1,
        .paramCount = PARAM_COUNT,
        .params = params
    };

    // Initialize parameters

    unitInfo.params[PARAM_TRIGGER] = (NullUnitParamInfo) {
        .name = "trigger",
        .value = false,
        .min = false,
        .max = true,
        .type = NULL_PARAM_BOOL
    };

    gen_midi_float("attack", &unitInfo.params[PARAM_ATTACK]);
    gen_midi_float("decay", &unitInfo.params[PARAM_DECAY]);
    gen_midi_float("sustain", &unitInfo.params[PARAM_SUSTAIN]);
    gen_midi_float("release", &unitInfo.params[PARAM_RELEASE]);

    unitInfo.params[PARAM_ATTACK].max.f =  5.0f;
    unitInfo.params[PARAM_ATTACK].value.f =  0.1f;

    unitInfo.params[PARAM_DECAY].max.f =  5.0f;
    unitInfo.params[PARAM_DECAY].value.f =  0.1f;

    unitInfo.params[PARAM_SUSTAIN].max.f =  1.0f;
    unitInfo.params[PARAM_SUSTAIN].value.f =  0.7f;

    unitInfo.params[PARAM_RELEASE].max.f =  5.0f;
    unitInfo.params[PARAM_RELEASE].value.f =  0.3f;

    return 0;
}

void destroy() {}

float process(uint8_t position, float input, uint8_t channel, float sampleRate, double currentTime) {
    if (isnan(input)) return 0.0f;

    bool newNoteOn = unitInfo.params[PARAM_TRIGGER].value.i;
    float attack = unitInfo.params[PARAM_ATTACK].value.f;
    float decay = unitInfo.params[PARAM_DECAY].value.f;
    float sustain = unitInfo.params[PARAM_SUSTAIN].value.f;
    float release = unitInfo.params[PARAM_RELEASE].value.f;

    // Note on trigger
    if (newNoteOn && !envelope.noteOn) {
        envelope.state = ATTACK;
        envelope.noteOn = true;
        envelope.noteOnTime = currentTime;
    }
    // Note off trigger
    else if (!newNoteOn && envelope.noteOn) {
        envelope.state = RELEASE;
        envelope.noteOn = false;
        envelope.noteOffTime = currentTime;
        envelope.releaseLevel = envelope.currentLevel;
    }

    // ADSR state machine
    double deltaTime;
    switch (envelope.state) {
        case ATTACK:
            deltaTime = currentTime - envelope.noteOnTime;
            if (deltaTime < attack) {
                envelope.currentLevel = deltaTime / attack;
            } else {
                envelope.state = DECAY;
            }
            break;

        case DECAY:
            deltaTime = currentTime - envelope.noteOnTime - attack;
            if (deltaTime < decay) {
                envelope.currentLevel = 1.0f - ((1.0f - sustain) * (deltaTime / decay));
            } else {
                envelope.state = SUSTAIN;
                envelope.currentLevel = sustain;
            }
            break;

        case SUSTAIN:
            envelope.currentLevel = sustain;
            break;

        case RELEASE:
            deltaTime = currentTime - envelope.noteOffTime;
            if (deltaTime < release) {
                envelope.currentLevel = envelope.releaseLevel * (1.0f - (deltaTime / release));
            } else {
                envelope.state = IDLE;
                envelope.currentLevel = 0.0f;
            }
            break;

        case IDLE:
            envelope.currentLevel = 0.0f;
            break;
    }

    return envelope.currentLevel * input;
}

NullUnitnInfo* get_info() {
    return &unitInfo;
}

void param_set(uint8_t paramId, NullUnitParamValue* value) {
    if (paramId >= PARAM_COUNT) {
        return;
    }
    unitInfo.params[paramId].value = *value;
}

NullUnitParamValue* param_get(uint8_t paramId) {
    if (paramId >= PARAM_COUNT) {
        return NULL;
    }
    return &unitInfo.params[paramId].value;
}
