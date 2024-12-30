This implements a headless native host for null-units. It's not at all ready, I am still working on it.

### features

- OSC messages (over udp) automatically tied to every unit for live-control
- presets (bundles) for loading, routing, and initial parameters
- change unit dir from CLI options
- preload data (samples, etc) from CLI options

### usage

```bash
# use docs/units to find units, preload samples like on web example, load bundle to connect everything
./native/build/nullunits -u docs/units -d samples/sin.raw -d samples/sqr.raw -d samples/tri.raw -d samples/saw.raw -b example.bundle
```

### todo

- handle websocket OSC for web clients
- save bundle of current state
- load bundle outside of intiial (OSC command)
