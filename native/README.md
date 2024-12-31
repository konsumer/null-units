This implements a headless native host for null-units. It's not at all ready, I am still working on it.

### features

- OSC messages (over udp) automatically tied to every unit for live-control
- presets (bundles) for loading, routing, and initial parameters
- change unit dir from CLI options
- preload data (samples, etc) from CLI options

### usage

```bash
# use docs/units to find units, load bundle to connect everything
./native/build/nullunits -u docs/units -b example.bundle

# in addition to built-in "simple" samples, you can load raw PCM (maybe more formats later)
# since built-in samples are 0-3, your samples will start at id 4
./native/build/nullunits -u docs/units -d samples/whatever.raw
```

### todo

- handle websocket OSC for web clients
- save bundle of current state
- load bundle outside of intiial (OSC command)
