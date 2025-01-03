The idea with this is inspired by [Korg's logue API](https://korginc.github.io/logue-sdk/) and [puredata](https://puredata.info/).

[![intro video](https://img.youtube.com/vi/69S7uBG3oiI/0.jpg)](https://youtu.be/69S7uBG3oiI)

Essentially, "units" are things that produce or process sound, and they can be strung together to create instrumnents. They are made in wasm, and can be written in any programming language that compiles to that.

Once a unit is compiled, it can be shared between multiple frames (think of this as a "device".) Here are some examples:

- [web-based editor](http://konsumer.js.org/null-units/)
- a simpler, more musical demo [on the web](https://konsumer.js.org/null-units/rickroll.htm)
- a similar example that plays [mario](https://konsumer.js.org/null-units/mario.htm)
- pi-zero handheld device with small monochrome OLED screen and MIDI input
- ESP32?
- embed engine in other things (game engine?) with wasm or native code.

The [NTS-3](https://www.korg.com/us/products/dj/nts_3/) as an example device, has only X/Y/depth params that can be tied to any param (max 8 each unit) of 4 units, and then units can be routed to each other. This will do similar, but allows more params, and units can be shared between any device, and written in any language.

## tools

```bash
# live-reloading webserver
npm start

# delete built files
npm run clean

# just build units
npm run build
```

## available units

You can read lots more about what is already made [here](UNITS.md). There is a lot of room for improvment, and I would love more, especially higher-level full units (complete emulations of machines and stuff.) PRs are definitely welcome.


## unit API

There are only a few functions to export. Implement those in your unit, and it will work in the rest of the ecosystem. I am using C here, but you can use anything you like.

Things are in flux right now, so check [null-unit.h](units/null-unit.h) to see what is available.

The essential plan is basic WASI preview1, without files.


## ideas/todo

- storage format loading several units, and their routing & initial values (to share instruments made up of units between things.) I am leaning towards OSC bundles of plain messages (laod units, set params, connect things, etc.)
- normalize all time to float seconds, frequencies/percents are midi (0-127)
- fix UI reactivity. currently it listens for change and the update is a bit jenk. Maybe it could be fixed with web-components, or better coupling
- make a puredata frame for these, so you can play with them there, like `[null-unit~ oscillator]` (to load oscillator.wasm from central dir)
- make a wrapper for ladspa, so I can just compile those directly to units
