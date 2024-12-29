#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <lo/lo.h>

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
int handle_int(const char *path, const char *types, lo_arg **argv,
              int argc, lo_message msg, void *user_data) {
    if (argc < 1) return 0;

    int value = argv[0]->i;
    printf("\nReceived %s: %d (0x%08x)\n", path, value, value);

    // Send response back
    lo_send(client_address, "/response/int", "i", value);
    printf("Sent response: %d (0x%08x)\n", value, value);

    return 0;
}

// Error handler
void error_handler(int num, const char *msg, const char *path) {
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

int main(int argc, char *argv[]) {
    int in_port = 53100;  // default input port
    int out_port = 53101; // default output port
    char port_str[16];

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in_port") == 0) {
            if (i + 1 < argc) {
                in_port = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out_port") == 0) {
            if (i + 1 < argc) {
                out_port = atoi(argv[++i]);
            }
        }
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
    client_address = lo_address_new("127.0.0.1", (const char *)&out_port);
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
