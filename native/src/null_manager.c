#include "null_manager.h"
#include "samples.h"

// TODO: put these inside manager
static struct SoundIo *soundio;
static struct SoundIoDevice *device;

// example, just to make sure sound is working
static float seconds_offset = 0.0f;
static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
  NullUnitManager *manager = (NullUnitManager*)outstream->userdata;
  (void)manager;

  const struct SoundIoChannelLayout *layout = &outstream->layout;
     float float_sample_rate = outstream->sample_rate;
     float seconds_per_frame = 1.0f / float_sample_rate;
     struct SoundIoChannelArea *areas;
     int frames_left = frame_count_max;
     int err;

     while (frames_left > 0) {
         int frame_count = frames_left;

         if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
             fprintf(stderr, "%s\n", soundio_strerror(err));
             exit(1);
         }

         if (!frame_count)
             break;

         float pitch = 440.0f;
         float radians_per_second = pitch * 2.0f * M_PI;
         for (int frame = 0; frame < frame_count; frame += 1) {
             float sample = sinf((seconds_offset + frame * seconds_per_frame) * radians_per_second);
             for (int channel = 0; channel < layout->channel_count; channel += 1) {
                 float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                 *ptr = sample;
             }
         }
         seconds_offset = fmodf(seconds_offset +
             seconds_per_frame * frame_count, 1.0f);

         if ((err = soundio_outstream_end_write(outstream))) {
             fprintf(stderr, "%s\n", soundio_strerror(err));
             exit(1);
         }

         frames_left -= frame_count;
     }
}

// run on every audio-frame (put in your update-loop)
void null_manager_process() {
  soundio_wait_events(soundio);
}


// Initialize the audio system and manager
NullUnitManager* null_manager_create() {
  NullUnitManager* manager =  malloc(sizeof(NullUnitManager));
  manager->units = NULL;
  manager->samples = NULL;
  manager->available_units = NULL;

  soundio = soundio_create();
  if (!soundio) {
      fprintf(stderr, "Out of memory\n");
      null_manager_destroy(manager);
      return NULL;
  }

  int err = soundio_connect(soundio);
  if (err) {
      fprintf(stderr, "Error connecting: %s\n", soundio_strerror(err));
      null_manager_destroy(manager);
      return NULL;
  }

  soundio_flush_events(soundio);

  int default_out_device_index = soundio_default_output_device_index(soundio);
  if (default_out_device_index < 0) {
      fprintf(stderr, "No output device found\n");
      null_manager_destroy(manager);
      return NULL;
  }

  device = soundio_get_output_device(soundio, default_out_device_index);
  if (!device) {
      fprintf(stderr, "Out of memory\n");
      null_manager_destroy(manager);
      return NULL;
  }

  manager->outstream = soundio_outstream_create(device);
  if (!manager->outstream) {
    fprintf(stderr, "Out of memory\n");
    null_manager_destroy(manager);
    return NULL;
  }

  manager->outstream->format = SoundIoFormatFloat32NE;
  manager->outstream->sample_rate = SAMPLE_RATE;
  manager->outstream->write_callback = write_callback;
  manager->outstream->userdata = &manager;

  if ((err = soundio_outstream_open(manager->outstream))) {
    fprintf(stderr, "Unable to open device: %s\n", soundio_strerror(err));
    null_manager_destroy(manager);
    return NULL;
  }

  if ((err = soundio_outstream_start(manager->outstream))) {
    fprintf(stderr, "Unable to start device: %s\n", soundio_strerror(err));
    null_manager_destroy(manager);
    return NULL;
  }

  // index 0 is audioOut
  NullUnit* audioOut = malloc(sizeof(NullUnit));
  audioOut->active = true;
  audioOut->info = malloc(sizeof(NullUnitnInfo));
  audioOut->info->name = strdup("out");
  audioOut->info->channelsIn = 1;
  audioOut->info->channelsOut = 0;
  audioOut->info->params = NULL;

  // TODO: setup input_buffer/output_buffer for osc/audioOut

  cvector_push_back(manager->units, audioOut);

  // track built-ins
  NullUnitAvailable unitForList = (NullUnitAvailable){
    .name="out",
    .path=NULL
  };
  cvector_push_back(manager->available_units, unitForList);
  unitForList.name = strdup("osc");
  cvector_push_back(manager->available_units, unitForList);

  // load built-in samples
  NullUnitSample sample = { .len=samples_sin_raw_len, .data=(float*)samples_sin_raw };
  cvector_push_back(manager->samples, sample);

  sample.data = (float*)samples_sqr_raw;
  cvector_push_back(manager->samples, sample);

  sample.data = (float*)samples_tri_raw;
  cvector_push_back(manager->samples, sample);

  sample.data = (float*)samples_saw_raw;
  cvector_push_back(manager->samples, sample);

  return manager;
}

// Clean up
void null_manager_destroy(NullUnitManager* manager) {
  if (manager->outstream != NULL){
    soundio_outstream_destroy(manager->outstream);
  }
  if (device != NULL) {
    soundio_device_unref(device);
  }
  if (soundio != NULL) {
    soundio_destroy(soundio);
  }

  // TODO: free all manager->units, manager->samples
  // TODO: free wasm stuff
  free(manager);
}

// load a unit
unsigned int null_manager_load(NullUnitManager* manager, const char* name) {
  unsigned int newUnitId = cvector_size(manager->units);
  // TODO: do wasm stuff
  return newUnitId;
}

// unload a unit
void null_manager_unload(NullUnitManager* manager, unsigned int unitId) {
  // TODO: do wasm stuff
}

// connect a unit to another
void null_manager_connect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {
  // TODO: do audio stuff
}

// disconnect
void null_manager_disconnect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {
  // TODO: do audio stuff
}

// set a param of a unit
void null_manager_set_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId, NullUnitParamValue value, float timefromNowInSeconds) {
  // TODO: do wasm stuff
}

// get a param of a unit
NullUnitParamValue* null_manager_get_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId) {
  // TODO: do wasm stuff
  return NULL;
}

// get info about a loaded unit
NullUnitnInfo* null_manager_get_info(NullUnitManager* manager, unsigned int unitSourceId) {
  // TODO: do wasm stuff
  return NULL;
}

// load list of wasm files in a dir into manager->available_units
void null_manager_get_units(NullUnitManager* manager, const char* dirname) {
    glob_t glob_result;
    char pattern[1024];

    // Create the glob pattern by combining dirname with "/*.wasm"
    snprintf(pattern, sizeof(pattern), "%s/*.wasm", dirname);

    // Perform the glob operation
    int ret = glob(pattern, GLOB_TILDE, NULL, &glob_result);
    if (ret != 0) {
        // Handle error if needed
        return;
    }

    // Iterate through all found files
    for (size_t i = 0; i < glob_result.gl_pathc; i++) {
        char* full_path = strdup(glob_result.gl_pathv[i]);
        char* name = strdup(basename(glob_result.gl_pathv[i]));

        // Remove .wasm extension from name
        char* dot = strrchr(name, '.');
        if (dot != NULL) {
            *dot = '\0';
        }

        // Create new NullUnitAvailable structure
        NullUnitAvailable unit = {
            .name = name,
            .path = full_path
        };

        // Add to the vector
        cvector_push_back(manager->available_units, unit);
    }

    // Free the glob structure
    globfree(&glob_result);
}

// just read a file as bytes
unsigned char* null_manager_read_file(char* filename, int* bytesRead) {
    FILE* file;
    unsigned char* buffer;
    long file_size;

    // Open the file in binary read mode
    file = fopen(filename, "rb");
    if (file == NULL) {
        *bytesRead = 0;
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // Allocate memory for the buffer
    buffer = (unsigned char*)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        *bytesRead = 0;
        return NULL;
    }

    // Read the file into the buffer
    *bytesRead = fread(buffer, 1, file_size, file);

    // Close the file
    fclose(file);

    // Check if we read the expected number of bytes
    if (*bytesRead != file_size) {
        free(buffer);
        return NULL;
    }

    return buffer;
}
