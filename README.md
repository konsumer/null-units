The idea with this is inspired by [Korg's logue API](https://korginc.github.io/logue-sdk/) and [puredata](https://puredata.info/).

Essentially, "units" are things that produce or process sound, and they can be strung together to create instrumnents. They are made in wasm, and can be written in any programming language that compiles to that.

Once a unit is compiled, it can be shared between multiple frames (think of this as a "device".) Here are some examples:

- [web-based editor](http://konsumer.js.org/null-units/)
- pi-zero handheld device with OLED screen and MIDI input
- ESP32?

The [NTS-3](https://www.korg.com/us/products/dj/nts_3/) as an example device, has only X/Y/depth params that can be tied to any param (max 8 each unit) of 4 units, and then units can be routed to each other. This will do similar, but allows more params, and units can be shared between any device, and written in any language.


## unit API

There are only a few functions to export. Implement those in your unit, and it will work in the rest of the ecosystem. I am using C here, but you can use anything you like.

```c
// called when the unit is loaded, set initialParams to NULL, if you want
void init(unsigned int initialParams[]);

// process a single value, in a 0-255 position frame, return output
float process(unsigned char position, float input, unsigned char channel);

// called when you plugin is unloaded
void destroy();

// get param count
unsigned int get_param_count();

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
void get_bytes(unsigned int id, unsigned int offset, unsigned int length, unsigned int* out);

// memory management
void* malloc(size_t size);
void free(void *ptr);
void* memset(void* ptr, int value, size_t num);

// utils for debugging
void join_strings(char *dest, const char *src);
void itoa(int num, char* str);
void ftoa(float num, char* str, int precision);
```

Additionally, host provides the [math.h functions](https://en.wikipedia.org/wiki/C_mathematical_functions), just because they are needed for a lot of audio-math, and it's nice to not need any other headers.

### ideas

- make a puredata frame for these, so you can play with them there, like `[null-unit~ oscillator]` (to load oscillator.wasm from central dir)
- storage format loading several units, and their routing & initial values (to share instruments made up of units between things)
