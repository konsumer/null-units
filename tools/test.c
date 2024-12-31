// this is a basic test of engine, without osc/cli stuff

#include <signal.h>
#include "null_manager.h"

static int keep_running = 1;

// Signal handler for Ctrl+C
void signal_handler(int signum) {
  printf("\nExiting...\n");
  keep_running = 0;
}

int main() {
  signal(SIGINT, signal_handler);

  NullUnitManager* manager = null_manager_create();

  // print available units
  null_manager_get_units("docs/units", manager);
  int i=0;
  int c = cvector_size(manager->available_units);
  if (c > 0) {
    printf("units:\n");
    for (i=0; i<c; i++) {
      printf("  %s: %s\n", manager->available_units[i].name, manager->available_units[i].path);
    }
  }

  NullUnitParamValue value = {};

  unsigned int audioOut = 0;
  unsigned int osc = null_manager_load(manager, "osc");
  null_manager_connect(manager, osc, audioOut, 0, 0);
  printf("\nosc -> audioOut: %u -> %u\n", osc, audioOut);

  value.i = 1;
  null_manager_set_param(manager, osc, 0, value, 0.0f);
  printf("osc type (0) set to 1 (sqr)\n");

  value.f = 60.0f;
  null_manager_set_param(manager, osc, 1, value, 0.0f);
  printf("osc note (1) set to 60\n");


  while(keep_running) {
  }

  null_manager_destroy(manager);
  return 0;
}
