#!/usr/bin/env python3

# this is a simple example OSC server to test that the protocol is working correctly

from pythonosc import udp_client
from pythonosc.osc_server import BlockingOSCUDPServer
from pythonosc.dispatcher import Dispatcher
import threading
import time
import argparse

def handle_int(address, *args):
    if not args:
        return

    value = args[0]
    print(f"\nReceived {address}: {value} (0x{value:08x})")

    # Send response back
    client.send_message("/response/int", value)
    print(f"Sent response: {value} (0x{value:08x})")

class DebugOSCServer(BlockingOSCUDPServer):
    def handle_request(self):
        try:
            data, client_address = self.socket.recvfrom(65535)
            print("\nRaw received data:", ' '.join(f'{b:02x}' for b in data))
            super().handle_request()
        except Exception as e:
            print(f"Error handling message: {e}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inport', type=int, default=53100, help='UDP port to receive messages on (default: 53100)')
    parser.add_argument('-o', '--outport', type=int, default=53101, help='UDP port to send responses on (default: 53101)')
    args = parser.parse_args()

    # Setup dispatcher
    dispatcher = Dispatcher()
    dispatcher.map("/test/int", handle_int)

    # Setup server
    server = DebugOSCServer(("127.0.0.1", args.in_port), dispatcher)

    # Setup client for responses
    global client
    client = udp_client.SimpleUDPClient("127.0.0.1", args.out_port)

    print(f"OSC Server listening on port {args.in_port}")
    print(f"Sending responses to port {args.out_port}")
    print("Press Ctrl+C to exit\n")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nExiting...")
        server.shutdown()

if __name__ == "__main__":
    main()
