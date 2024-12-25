#include "null_manager.h"

// Forward declare our callback
static void get_data_floats(wasm_exec_env_t exec_env, unsigned int id, unsigned int offset, unsigned int length, float* out);


static NativeSymbol native_symbols[] = {
    {
        "get_data_floats",
        (void *)get_data_floats,
        "(iiiF)v",
        NULL
    }
};

// Callback function for WASM units to get sample data
void get_data_floats(wasm_exec_env_t exec_env, unsigned int id, unsigned int offset, unsigned int length, float* out) {
    NullManager* manager = wasm_runtime_get_user_data(exec_env);
    if (id < 4 && manager->samples[id]) {
        memcpy(out, manager->samples[id] + offset, length * sizeof(float));
    }
}

static void write_callback(struct SoundIoOutStream* outstream,
                         int frame_count_min, int frame_count_max) {
    NullManager* manager = outstream->userdata;
    struct SoundIoChannelArea* areas;
    int frames_left = frame_count_max;
    int err;

    // Create a constant for sample rate
    const float sample_rate = (float)SAMPLE_RATE;

    while (frames_left > 0) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "write error: %s", soundio_strerror(err));
            return;
        }

        // Process audio through the chain of units
        for (int frame = 0; frame < frame_count; frame++) {
            float sample = 0.0f;
            // Process through active units
            for (unsigned int i = 1; i < manager->unit_count; i++) {
                if (manager->units[i].active) {
                    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(
                        manager->units[i].module_inst, 8192);

                    // Get the process function
                    wasm_function_inst_t func = wasm_runtime_lookup_function(
                        manager->units[i].module_inst, "process");

                    if (func) {
                        uint32_t argv[] = {
                            (uint32_t)frame,          // position
                            *(uint32_t*)&sample,      // input
                            0,                        // channel
                            *(uint32_t*)&sample_rate, // sampleRate
                            0                         // currentTime (simplified)
                        };

                        // Call the WASM function
                        const uint32_t argc = 5;  // number of arguments
                        if (!wasm_runtime_call_wasm(exec_env, func, argc, argv)) {
                            fprintf(stderr, "call wasm function failed: %s\n",
                                    wasm_runtime_get_exception(manager->units[i].module_inst));
                        } else {
                            // Get the return value
                            sample = *(float*)wasm_runtime_get_custom_data(exec_env);
                        }
                    }
                    wasm_runtime_destroy_exec_env(exec_env);
                }
            }

            // Write to output
            float* ptr = (float*)(areas[0].ptr + areas[0].step * frame);
            *ptr = sample;
        }

        if ((err = soundio_outstream_end_write(outstream))) {
            fprintf(stderr, "end write error: %s", soundio_strerror(err));
            return;
        }

        frames_left -= frame_count;
    }
}

NullManager* null_manager_create(void) {
    NullManager* manager = calloc(1, sizeof(NullManager));
    manager->soundio = soundio_create();

    int err;
    if ((err = soundio_connect(manager->soundio))) {
        fprintf(stderr, "error connecting: %s", soundio_strerror(err));
        return NULL;
    }

    soundio_flush_events(manager->soundio);

    int default_out_device_index = soundio_default_output_device_index(manager->soundio);
    manager->device = soundio_get_output_device(manager->soundio, default_out_device_index);

    manager->outstream = soundio_outstream_create(manager->device);
    manager->outstream->format = SoundIoFormatFloat32NE;
    manager->outstream->write_callback = write_callback;
    manager->outstream->sample_rate = SAMPLE_RATE;
    manager->outstream->userdata = manager;

    if ((err = soundio_outstream_open(manager->outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return NULL;
    }

    if ((err = soundio_outstream_start(manager->outstream))) {
        fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
        return NULL;
    }

    int n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    if (!wasm_runtime_register_natives("env", native_symbols, n_native_symbols)) {
        fprintf(stderr, "Failed to register native symbols\n");
        return NULL;
    }

    // Initialize WASM runtime
    if (!wasm_runtime_init())
        return NULL;

    return manager;
}

unsigned int load(NullManager* manager, const char* name) {
    if (manager->unit_count >= MAX_UNITS) {
        fprintf(stderr, "Maximum number of units reached\n");
        return 0;
    }

    // Construct the path to the WASM file
    char path[256];
    snprintf(path, sizeof(path), "%s.wasm", name);

    // Read the WASM file
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open WASM file: %s\n", path);
        return 0;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read file content
    uint8_t* wasm_buf = malloc(file_size);
    if (fread(wasm_buf, 1, file_size, file) != file_size) {
        fprintf(stderr, "Failed to read WASM file\n");
        fclose(file);
        free(wasm_buf);
        return 0;
    }
    fclose(file);

    // Load WASM module
    char error_buf[128];
    wasm_module_t* module = wasm_runtime_load(wasm_buf, file_size, error_buf, sizeof(error_buf));
    if (!module) {
        fprintf(stderr, "Failed to load WASM module: %s\n", error_buf);
        free(wasm_buf);
        return 0;
    }

    // Instantiate the module
    wasm_module_inst_t* module_inst = wasm_runtime_instantiate(module, 8192, 8192, error_buf, sizeof(error_buf));
    if (!module_inst) {
        fprintf(stderr, "Failed to instantiate WASM module: %s\n", error_buf);
        wasm_runtime_unload(module);
        free(wasm_buf);
        return 0;
    }

    // Store the unit
    unsigned int new_id = manager->unit_count++;
    manager->units[new_id].module = module;
    manager->units[new_id].module_inst = module_inst;
    manager->units[new_id].active = true;

    // Get unit info
    wasm_function_inst_t get_info_func = wasm_runtime_lookup_function(module_inst, "get_info");
    if (get_info_func) {
        if (!wasm_runtime_call_wasm(wasm_runtime_create_exec_env(module_inst, 8192),
                                  get_info_func, 0, NULL)) {
            fprintf(stderr, "Failed to get unit info\n");
        } else {
            manager->units[new_id].info = (NullUnitnInfo*)wasm_runtime_get_custom_data(module_inst);
        }
    }

    free(wasm_buf);
    return new_id;
}

void set_param(NullManager* manager, unsigned int unitId, unsigned int paramId,
              NullUnitParamValue value, unsigned int timefromNowInSeconds) {
    if (unitId >= manager->unit_count || !manager->units[unitId].active) {
        fprintf(stderr, "Invalid unit ID\n");
        return;
    }

    NullUnit* unit = &manager->units[unitId];

    // Get the param_set function
    wasm_function_inst_t func = wasm_runtime_lookup_function(unit->module_inst, "param_set");
    if (!func) {
        fprintf(stderr, "param_set function not found\n");
        return;
    }

    // Prepare arguments
    uint32_t argv[] = {
        paramId,
        (uint32_t)&value  // Pass pointer to the value
    };

    // Call the function
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(unit->module_inst, 8192);
    if (!wasm_runtime_call_wasm(exec_env, func, 2, argv)) {
        fprintf(stderr, "Failed to set parameter: %s\n",
                wasm_runtime_get_exception(unit->module_inst));
    }
    wasm_runtime_destroy_exec_env(exec_env);
}

void null_manager_destroy(NullManager* manager) {
    if (!manager) return;

    // Stop and clean up audio
    if (manager->outstream) {
        soundio_outstream_destroy(manager->outstream);
    }
    if (manager->device) {
        soundio_device_unref(manager->device);
    }
    if (manager->soundio) {
        soundio_destroy(manager->soundio);
    }

    // Clean up all units
    for (unsigned int i = 0; i < manager->unit_count; i++) {
        if (manager->units[i].active) {
            if (manager->units[i].module_inst) {
                wasm_runtime_deinstantiate(manager->units[i].module_inst);
            }
            if (manager->units[i].module) {
                wasm_runtime_unload(manager->units[i].module);
            }
            // Free any other unit-specific resources here
        }
    }

    // Clean up WASM runtime
    wasm_runtime_destroy();

    // Free the manager itself
    free(manager);
}
