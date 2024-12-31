#include "null_manager.h"

// Initialize the audio system and manager
NullUnitManager* null_manager_create() {
  // TODO: setup soundIO

  NullUnitManager* manager =  malloc(sizeof(NullUnitManager));
  manager->units = NULL;
  manager->samples = NULL;
  manager->available_units = NULL;

  // index 0 is audioOut
  NullUnit* audioOut = malloc(sizeof(NullUnit));
  audioOut->active = true;
  audioOut->info = malloc(sizeof(NullUnitnInfo));
  audioOut->info->name = strdup("out");
  audioOut->info->channelsIn = 1;
  audioOut->info->channelsOut = 0;
  audioOut->info->params = NULL;

  // TODO: setup input_buffer/output_buffer

  cvector_push_back(manager->units, audioOut);

  // track built-ins
  NullUnitAvailable unitForList = (NullUnitAvailable){
    .name="out",
    .path=NULL
  };
  cvector_push_back(manager->available_units, unitForList);
  unitForList.name = strdup("osc");
  cvector_push_back(manager->available_units, unitForList);


  return manager;
}

// Clean up
void null_manager_destroy(NullUnitManager* manager) {
  // TODO: free all manager->units, manager->samples
  free(manager);
}

// load a unit
unsigned int null_manager_load(NullUnitManager* manager, const char* name) {
  unsigned int newUnitId = cvector_size(manager->units);
  return newUnitId;
}

// unload a unit
void null_manager_unload(NullUnitManager* manager, unsigned int unitId) {

}

unsigned int null_manager_sample_load(NullUnitManager* manager, char* fileBasename) {
  unsigned int newSampleId = cvector_size(manager->samples);
  return newSampleId;
}

// connect a unit to another
void null_manager_connect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {

}

// disconnect
void null_manager_disconnect(NullUnitManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {

}

// set a param of a unit
void null_manager_set_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId, NullUnitParamValue value, float timefromNowInSeconds) {

}

// get a param of a unit
NullUnitParamValue* null_manager_get_param(NullUnitManager* manager, unsigned int unitSourceId, unsigned int paramId) {
  return NULL;
}

// get info about a loaded unit
NullUnitnInfo* null_manager_get_info(NullUnitManager* manager, unsigned int unitSourceId) {
  return NULL;
}

// get list of wasm files in a dir
void null_manager_get_units(const char* dirname, NullUnitManager* manager) {
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
