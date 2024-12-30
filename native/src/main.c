#include <getopt.h>
#include <lo/lo.h>
#include <signal.h>
#include "null_manager.h"

static lo_server server = NULL;
static lo_address client_address = NULL;
static int keep_running = 1;

// Signal handler for Ctrl+C
void signal_handler(int signum) {
  printf("\nExiting...\n");
  keep_running = 0;
}

void error_handler(int num, const char *msg, const char *path) {
  fprintf(stderr, "nullunit OSC server error %d in path %s: %s\n", num, path, msg);
}

// Handler for /unit/load messages
int handle_unit_load(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void* managerPtr) {
  if (argc != 1) {
    return 0;
  }

  char* name = &argv[0]->s;
  printf("unit load: %s\n", name);

  NullUnitManager* manager = (NullUnitManager*)managerPtr;
  unsigned int unitId = null_manager_load(manager, name);

  return lo_send(client_address, "/unit/load", "i", unitId);
}

// Handler for /unit/load messages
int handle_unit_unload(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void* managerPtr) {
  if (argc != 1) {
    return 0;
  }

  unsigned int unitId = argv[0]->i;

  printf("unit unload: %u\n", unitId);

  NullUnitManager* manager = (NullUnitManager*)managerPtr;
  null_manager_unload(manager, unitId);

  return 0;
}

// Handler for /unit/connect messages
int handle_unit_connect(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void* managerPtr) {
  if (argc != 4) {
    return 0;
  }

  unsigned int unitSourceId = argv[0]->i;
  unsigned int unitSourcePort = argv[1]->i;
  unsigned int unitDestinationId = argv[2]->i;
  unsigned int unitDestinationPort = argv[3]->i;
  printf("unit connect: %u %u %u %u\n", unitSourceId, unitSourcePort, unitDestinationId, unitDestinationPort);

  NullUnitManager* manager = (NullUnitManager*)managerPtr;
  null_manager_connect(manager, unitSourceId, unitSourcePort, unitDestinationId, unitDestinationPort);

  return 0;
}

// Handler for /unit/param messages (int value)
int handle_unit_param_i(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void* managerPtr) {
  if (argc != 4) {
    return 0;
  }

  unsigned int unitSourceId = argv[0]->i;
  unsigned int paramId = argv[1]->i;
  NullUnitParamValue value = { .i=argv[2]->i };
  float timefromNowInSeconds = argv[3]->f;
  printf("unit param: %u %u %d %f\n", unitSourceId, paramId, value.i, timefromNowInSeconds);

  NullUnitManager* manager = (NullUnitManager*)managerPtr;
  null_manager_set_param(manager, unitSourceId, paramId, value, timefromNowInSeconds);

  return 0;
}

// Handler for /unit/param messages (float value)
int handle_unit_param_f(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void* managerPtr) {
  if (argc != 4) {
    return 0;
  }

  unsigned int unitSourceId = argv[0]->i;
  unsigned int paramId = argv[1]->i;
  NullUnitParamValue value = { .f=argv[2]->f };
  float timefromNowInSeconds = argv[3]->f;
  printf("unit param: %u %u %f %f\n", unitSourceId, paramId, value.f, timefromNowInSeconds);

  NullUnitManager* manager = (NullUnitManager*)managerPtr;
  null_manager_set_param(manager, unitSourceId, paramId, value, timefromNowInSeconds);

  return 0;
}

void print_usage() {
  printf("Usage: nullunit [options]\n");
  printf("Options:\n");
  printf("  -i, --inport PORT   UDP port to receive messages on (default: 53100)\n");
  printf("  -o, --outport PORT  UDP port to send responses on (default: 53101)\n");
  printf("  -u, --unit DIR      Directory path to find wasm-units - multiple ok\n");
  printf("  -b, --bundle FILE   File path to load bundle - multiple ok\n");
  printf("  -d, --data FILE     File path to load data (sample) - multiple ok\n");
}

int main(int argc, char *argv[]) {
  int in_port = 53100;
  int out_port = 0;
  NullUnitManager* manager = null_manager_create();
  cvector_vector_type(char*) unitPaths = NULL;
  cvector_vector_type(char*) bundles = NULL;
  cvector_vector_type(char*) dataFiles = NULL;

  // Long options
  static struct option long_options[] = {
    { "outport", required_argument, 0, 'o' },
    { "inport", required_argument, 0, 'i' },
    { "unit", required_argument, 0, 'u' },
    { "bundle", required_argument, 0, 'b' },
    { "data", required_argument, 0, 'd' },
    { 0, 0, 0, 0 }
  };

  // Parse command line options
  int opt;
  while ((opt = getopt_long(argc, argv, "o:i:u:b:d:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'o':
        out_port = atoi(optarg);
        break;
      case 'i':
        in_port = atoi(optarg);
        break;
      case 'u':
        cvector_push_back(unitPaths, optarg);
        break;
      case 'b':
        cvector_push_back(bundles, optarg);
        break;
      case 'd':
          cvector_push_back(dataFiles, optarg);
          break;
      default:
        print_usage();
        return 1;
    }
  }

  if (out_port == 0) {
    out_port = in_port + 1;
  }

  signal(SIGINT, signal_handler);

  // Create OSC server
  char in_port_str[16];
  snprintf(in_port_str, sizeof(in_port_str), "%d", in_port);
  server = lo_server_new(in_port_str, error_handler);
  if (!server) {
    fprintf(stderr, "Could not create server\n");
    return 1;
  }

  // Setup client address for responses
  char out_port_str[16];
  snprintf(out_port_str, sizeof(out_port_str), "%d", out_port);
  client_address = lo_address_new("127.0.0.1", out_port_str);

  lo_server_add_method(server, "/unit/load", "s", handle_unit_load, manager);
  lo_server_add_method(server, "/unit/connect", "iiii", handle_unit_connect, manager);
  lo_server_add_method(server, "/unit/unload", "i", handle_unit_unload, manager);


  // depends on incoming type
  lo_server_add_method(server, "/unit/param", "iiif", handle_unit_param_i, manager);
  lo_server_add_method(server, "/unit/param", "iiff", handle_unit_param_f, manager);

  int i = 0;
  int c = cvector_size(unitPaths);
  if (c > 0) {
    printf("unit paths:\n");
    for (i=0; i<c; i++) {
      printf("  %s\n", unitPaths[i]);
      get_units_in_dir(unitPaths[i], &manager->available_units);
    }
  }
  c = cvector_size(manager->available_units);
  if (c > 0) {
    printf("units:\n");
    for (i=0; i<c; i++) {
      printf("  %s: %s\n", manager->available_units[i].name, manager->available_units[i].path);
    }
  }

  // TODO: for each bundle, run messages
  c = cvector_size(bundles);
  if (c > 0) {
    printf("bundles:\n");
    for (i=0; i<c; i++) {
      printf("  %s\n", bundles[i]);
    }
  }

  // TODO: currently only supports raw samples, but I could load audio files...
  c = cvector_size(dataFiles);
  if (c > 0) {
    printf("data:\n");
    int bytesLen;
    int sampleId = 0;
    for (i=0; i<c; i++) {
      unsigned char* data = read_file(dataFiles[i], &bytesLen);
      if (data != NULL && bytesLen > 0) {
        printf("  %d: %s\n", sampleId, dataFiles[i]);
        NullUnitSample sample = {
          .data = (float*)data,
          .len = bytesLen
        };
        sampleId++;
        cvector_push_back(manager->samples, sample);
      } else {
        printf("  X: %s (not loaded)\n", dataFiles[i]);
      }
    }
  }

  printf("nullunit OSC Server running on ports %d/%d\n", in_port, out_port);
  printf("Press Ctrl+C to exit\n");

  while (keep_running) {
    lo_server_recv_noblock(server, 100);
  }

  lo_address_free(client_address);
  lo_server_free(server);
  null_manager_destroy(manager);

  return 0;
}
