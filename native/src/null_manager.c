#include "null_manager.h"

// Initialize the audio system and manager
NullManager* null_manager_create() {
  return NULL;
}

// Clean up
void null_manager_destroy(NullManager* manager) {

}

// load a unit
unsigned int null_manager_load(NullManager* manager, const char* name) {
  return 0;
}

// unload a unit
void null_manager_unload(NullManager* manager, unsigned int unitId) {

}

// connect a unit to another
void null_manager_connect(NullManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {

}

// disconnect
void null_manager_disconnect(NullManager* manager, unsigned int unitSourceId, unsigned int unitSourcePort, unsigned int unitDestinationId, unsigned int unitDestinationPort) {

}

// set a param of a unit
void null_manager_set_param(NullManager* manager, unsigned int unitSourceId, unsigned int paramId, NullUnitParamValue value, unsigned int timefromNowInSeconds) {

}

// get a param of a unit
NullUnitParamValue* null_manager_get_param(NullManager* manager, unsigned int unitSourceId, unsigned int paramId) {
  return NULL;
}

// get info about a loaded unit
NullUnitnInfo* null_manager_get_info(NullManager* manager, unsigned int unitSourceId) {
  return NULL;
}
