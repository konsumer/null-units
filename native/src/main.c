#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "null_manager.h"

static volatile bool keep_running = true;

static void interrupt_handler(int _) {
    (void)_;
    keep_running = false;
}

int main() {
    // Setup signal handling for clean exit
    signal(SIGINT, interrupt_handler);

    // Create our audio manager
    NullManager* manager = null_manager_create();
    if (!manager) {
        fprintf(stderr, "Failed to create audio manager\n");
        return 1;
    }

    // Load an oscillator unit
    unsigned int osc_id = load(manager, "osc");
    if (osc_id == 0) {
        fprintf(stderr, "Failed to load oscillator unit\n");
        null_manager_destroy(manager);
        return 1;
    }

    // Connect oscillator to output (unit 0)
    connect(manager, osc_id, 0, 0, 0);

    // Set oscillator parameters
    NullUnitParamValue value;

    // Set waveform type to sine (0)
    value.u = 0;
    set_param(manager, osc_id, 0, value, 0);

    // Set note to A4 (69 in MIDI)
    value.f = 69.0f;
    set_param(manager, osc_id, 1, value, 0);

    printf("Playing 440Hz sine wave. Press Ctrl+C to exit...\n");

    // Keep the program running until Ctrl+C
    while (keep_running) {
        soundio_flush_events(manager->soundio);
        soundio_wait_events(manager->soundio);
    }

    // Cleanup
    null_manager_destroy(manager);
    return 0;
}
