#!/usr/bin/env python3

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
            print("Raw received data:", ' '.join(f'{b:02x}' for b in data))
            super().handle_request()
        except Exception as e:
            print(f"Error handling message: {e}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--out_port', type=int, default=53100, help='Port to send to (default: 53100)')
    parser.add_argument('-i', '--in_port', type=int, default=53101, help='Port to receive on (default: 53101)')
    args = parser.parse_args()

    # Setup dispatcher
    dispatcher = Dispatcher()
    dispatcher.map("/response/int", handle_response)

    # Setup server to receive responses
    server = DebugOSCServer(("127.0.0.1", args.in_port), dispatcher)
    server_thread = threading.Thread(target=server.serve_forever)
    server_thread.daemon = True
    server_thread.start()

    # Setup client
    client = udp_client.SimpleUDPClient("127.0.0.1", args.out_port)

    print(f"Sending on port {args.out_port}")
    print(f"Receiving on port {args.in_port}")
    print("Press Ctrl+C to exit\n")

    try:
        counter = 0
        while True:
            value = counter % 256  # Keep values small for testing
            print(f"\nSending test value: {value} (0x{value:08x})")
            client.send_message("/test/int", value)
            counter += 1
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting...")
        server.shutdown()
        server_thread.join(timeout=1)

if __name__ == "__main__":
    main()
