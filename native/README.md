This implements a headless native host for null-units. It's not at all ready, I am still working on it.

### features

- OSC messages (over udp) automatically tied to every unit for live-control
- presets (message bundles) for loading, routing, and initial parameters
- preload data (samples, etc) from CLI options
- change unit dir from CLI options

### usage

```
Usage: nullunits [options]
Options:
  -i, --inport PORT   UDP port to receive messages on (default: 53100)
  -o, --outport PORT  UDP port to send responses on (default: 53101)
  -u, --unit DIR      Directory path to find wasm-units - multiple ok
  -b, --bundle FILE   File path to load bundle - multiple ok
  -d, --data FILE     File path to load data (sample) - multiple ok
```

For `multiple ok` options, they are processed in order.

#### examples

```bash
# use docs/units dir to find units, load bundle to connect everything
./native/build/nullunits -u docs/units -b example.bundle

# in addition to built-in "simple" samples, you can load raw PCM (maybe more formats later)
# since built-in samples are 0-3, your samples will start at id 4
./native/build/nullunits -u docs/units -d samples/whatever.raw
```

### todo

- handle websocket OSC for web clients
- save bundle of current state
- load bundle outside of intiial (OSC command)
