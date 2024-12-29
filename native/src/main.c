#include <getopt.h>
#include <lo/lo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Global variables
lo_server server = NULL;
lo_address client_address = NULL;
int keep_running = 1;

// Signal handler for Ctrl+C
void signal_handler(int signum) {
  printf("\nExiting...\n");
  keep_running = 0;
}

// Handler for /test/int messages
int handle_int(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data) {
  if (argc < 1)
    return 0;

  int value = argv[0]->i;
  printf("\nReceived %s: %d (0x%08x)\n", path, value, value);

  // Send response back and check return value
  int ret = lo_send(client_address, "/response/int", "i", value);
  if (ret < 0) {
    printf("Error sending response\n");
  } else {
    printf("Sent response: %d (0x%08x) [%d bytes sent]\n", value, value, ret);
  }

  return 0;
}

void error_handler(int num, const char *msg, const char *path) {
  printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

void print_usage() {
  printf("Usage: nullunit [options]\n");
  printf("Options:\n");
  printf("  -i, --inport PORT   UDP port to receive messages on (default: 53100)\n");
  printf("  -o, --outport PORT  UDP port to send responses on (default: 53101)\n");
}

int main(int argc, char *argv[]) {
  int in_port = 53100; // Default port to receive messages on
  int out_port = 0;    // Default port to send responses on
  char port_str[16];

  // Long options
  static struct option long_options[] = { { "outport", required_argument, 0, 'o' }, { "inport", required_argument, 0, 'i' }, { 0, 0, 0, 0 } };

  // Parse command line options
  int opt;
  while ((opt = getopt_long(argc, argv, "o:i:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'o':
      out_port = atoi(optarg);
      break;
    case 'i':
      in_port = atoi(optarg);
      break;
    default:
      print_usage();
      return 1;
    }
  }

  if (out_port == 0) {
    out_port = in_port + 1;
  }

  // Setup signal handler
  signal(SIGINT, signal_handler);

  // Convert input port to string
  snprintf(port_str, sizeof(port_str), "%d", in_port);

  // Create OSC server
  server = lo_server_new(port_str, error_handler);
  if (!server) {
    printf("Could not create server\n");
    return 1;
  }

  // Setup client address for responses
  char out_port_str[16];
  snprintf(out_port_str, sizeof(out_port_str), "%d", out_port);
  client_address = lo_address_new("127.0.0.1", out_port_str);

  if (!client_address) {
    printf("Could not create client address\n");
    lo_server_free(server);
    return 1;
  }

  // Add method handler
  lo_server_add_method(server, "/test/int", "i", handle_int, NULL);

  printf("OSC Server listening on port %d\n", in_port);
  printf("Sending responses to port %d\n", out_port);
  printf("Press Ctrl+C to exit\n\n");

  // Main loop
  while (keep_running) {
    lo_server_recv_noblock(server, 100);
  }

  // Cleanup
  lo_address_free(client_address);
  lo_server_free(server);

  return 0;
}
