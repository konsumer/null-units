#!/usr/bin/env python3

# this is a simple example OSC client to test that the protocol is working correctly

from pythonosc import udp_client
from pythonosc.osc_server import BlockingOSCUDPServer
from pythonosc.dispatcher import Dispatcher
import threading
import time
import argparse

def handle_response(address, *args):
    if not args:
        return
    value = args[0]
    print(f"\nReceived response {address}: {value} (0x{value:08x})")

class DebugOSCServer(BlockingOSCUDPServer):
    def handle_request(self):
        try:
            data, client_address = self.socket.recvfrom(65535)
            print(f"\nReceived raw data from {client_address}:")
            print("Data:", ' '.join(f'{b:02x}' for b in data))
            super().handle_request()
        except Exception as e:
            print(f"Error handling message: {e}")

def main():
    parser = argparse.ArgumentParser(description='OSC Client')
    parser.add_argument('-o', '--outport', type=int, default=53100, help='UDP port to connect to server (default: 53100)')
    parser.add_argument('-i', '--inport', type=int, default=53101, help='UDP port to receive responses on (default: 53101)')
    args = parser.parse_args()

    # Setup dispatcher
    dispatcher = Dispatcher()
    dispatcher.map("/*", handle_response)

    # Setup server to receive responses
    server = DebugOSCServer(("127.0.0.1", args.inport), dispatcher)
    print(f"Created UDP server listening on port {args.inport}")

    server_thread = threading.Thread(target=server.serve_forever)
    server_thread.daemon = True
    server_thread.start()
    print(f"Server thread started")

    # Setup client
    client = udp_client.SimpleUDPClient("127.0.0.1", args.outport)
    print(f"Created UDP client sending to port {args.outport}")

    print("\nConfiguration:")
    print(f"→ Sending to server on port {args.outport}")
    print(f"← Receiving responses on port {args.inport}")
    print("\nPress Ctrl+C to exit\n")

    try:
        # TODO: check on bundling (timestamps, multiple messages)
        client.send_message("/unit/load", "osc")
        client.send_message("/unit/connect", [1, 0, 0, 0])
        client.send_message("/unit/param", [1, 0, 1, 0.0]) # unit=1, param=0, value=1, time=0.0
        client.send_message("/unit/param", [1, 1, 60, 0.0]) # unit=1, param=1, value=60, time=0.0

        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting...")
        server.shutdown()
        server_thread.join(timeout=1)

if __name__ == "__main__":
    main()
