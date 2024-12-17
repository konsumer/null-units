The idea with this is inspired by [Korg's logue API](https://korginc.github.io/logue-sdk/).

Essentially, "units" are things that produce or process sound, and they can be strung together to create instrumnents. They are made in wasm, and can be written in any programming language that compiles to that.

Once a unit is compiled, it can be shared between multiple frames (think of this as a "device".) Here are some examples:

- [web-based editor](LINK_NEEDED)
- pi-zero handheld device with OLED screen and MIDI input
- ESP32?

The [NTS-3](https://www.korg.com/us/products/dj/nts_3/) as an example, has X/Y/depth params that can be tied to any param (max 8 each) of 4 units, and then routed to each other. This will do similar, but allows more params, and units can be shared beteween any device.


## unit API

There are only a few functions. Implement those in your unit, and it will work in the rest of the ecosystem. I am using C here, but you can use anything you like.

```c
// called when the unit is loaded, returns the number of params it accepts
unsigned int init();

// process a single value, in a 0-255 position frame, return output
float process(unsigned char position, float input, unsigned char channel);

// called when you plugin is unloaded
void destroy();

// set a parameter
void param_set(unsigned int param, unsigned int value);

// get the current value of a parameter
unsigned int param_get(unsigned int param);

// returns the name of the unit (32 characters, max)
char* get_name_unit();

// returns the name of the parameter (32 characters, max)
char* get_name_param(unsigned int param);

// returns number of channels
unsigned char get_channel_count();
```

The host also exposes a few functions:

```c
// get current unix time in ms (wraps around at 4.294967296e^9)
unsigned int now();

// get a pseudo-random number -3.4e^38 to 3.4e^38
float random();

// get some named bytes (sample, etc) from host
unsigned int get_bytes(char* name, unsigned int offset, unsigned int length);
```

Additionally, host provides the math.h functions, using floats instead of doubles (faster low-mem math.)


Additionally, some helpers will be provided for languages, when people make them. This means that I will probly have more tooling for C than assemblyscript, for exmaple, since I am making stuff using that.

### ideas

- make a puredata frame for these, so you can play with them there, like `[null-unit~ oscillator]` (to load oscillator.wasm from central dir)
- storage format loading several units, and their routing & initial values (to share instruments made up of units between things)
