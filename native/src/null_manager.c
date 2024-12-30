#include "null_manager.h"

// Initialize the audio system and manager
NullUnitManager* null_manager_create() {
  NullUnitManager* manager =  malloc(sizeof(NullUnitManager));
  manager->units = NULL;
  manager->samples = NULL;

  // index 0 is audioOut
  NullUnit* audioOut = malloc(sizeof(NullUnit));
  cvector_push_back(manager->units, audioOut);

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
