// this will load a null-unit wasm, as an AudioWorklet

import d from 'https://esm.sh/text-encoding-shim'
const { TextDecoder  } = d

const decoder = new TextDecoder()

class NullUnitProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.wasmInstance = null;
    this.position = 0;

    // this is shared collection of sample-data
    // 0 - sin
    // 1 - sqr
    // 2 - tri
    // 3 - saw

    this.data = {}
    const f = new Float32Array(256)
    // build some 256-length Float32Arrays for waveform data
    for (let w=0; w<4; w++) {
      for (let i = 0; i<256; i++) {
        const phase = i / 256;
        if (w==0) {
          // sine - already crosses zero
          f[i] = Math.sin(2 * Math.PI * phase)
        }
        if (w==1) {
          // square, offset to 0-crossing
          f[i] = phase < 0.5 ? 1.0 : -1.0
          if (i === 0 || i === 255) f[i] = 0
        }
        if (w==2) {
          // triangle - starts and ends at zero
          if (phase < 0.25) {
            f[i] = 4 * phase
          } else if (phase < 0.75) {
            f[i] = 2 - 4 * phase
          } else {
            f[i] = -4 + 4 * phase
          }
        }
        if (w==3) {
          // sawtooth - shift phase by 0.5 to start at zero
          f[i] = -1 + 2 * ((phase + 0.5) % 1)
        }
      }
      // copy the data into data[id]
      this.data[w] = new Float32Array(f.subarray(0, 256))
    }

    // this is host-functions exposed to the unit-wasm
    const importObject = {
      env: {
        trace(msgPtr) {
          console.log(this.getString(msgPtr))
        },

        get_data_floats(id, offset, length, out) {
          const mem = new Uint8Array(this.wasmInstance.memory.buffer)
          mem.set(new Uint8Array(this.data[id].buffer.slice(offset, offset + (length * 4))), out)
        },
      }
    }

    // bind importObject to worklet
    for (const k of Object.keys(importObject.env)) {
      importObject.env[k] = importObject.env[k].bind(this)
    }

    this.port.onmessage = async ({ data: { id, message }}) => {
      if (message.type === 'load') {
        try {
          this.wasmInstance = (await WebAssembly.instantiate(message.bytes, importObject)).instance.exports
        } catch (e) {
          console.error('Failed to load WASM module:', e);
        }

        // console.log('init', this.wasmInstance)
        if (this.wasmInstance.init) {
          this.wasmInstance.init()
        }

        // get info about the unit & return it
        const param_count = this.wasmInstance.get_param_count ? this.wasmInstance.get_param_count() : 0
        const response = {
          name: this.wasmInstance.get_name_unit ? this.getString(this.wasmInstance.get_name_unit()) : "",
          param_names: [],
          param_count
        }
        if (this.wasmInstance.get_param_name) {
          for (let i = 0; i < param_count; i++) {
            response.param_names.push(this.getString(this.wasmInstance.get_param_name(i)))
          }
        }
        this.port.postMessage({ id, response })
      }

      // allow parent thread to get a copy of sample-data
      if (message.type === 'get_sample') {
        this.port.postMessage({ id, response: this.data[message.index] })
      }

      // get the last id of samples (for adding new one)
      if (message.type === 'get_last_sample') {
        this.port.postMessage({ id, response: Object.keys(this.data).pop() })
      }

      // allow parent thread to set sample-data
      if (message.type === 'set_sample') {
        this.data[message.index] = message.data
        this.port.postMessage({ id, response: true })
      }

      // allow parent thread to set params
      if (message.type === 'param_set') {
        if (this.wasmInstance?.param_set) {
          this.wasmInstance.param_set(message.param, message.value)
        }
        this.port.postMessage({ id, response: true })
      }
    }
  }

  // get a string from wasmInstance memory
  getString(p, maxLength = 1024) {
    let len = 0
    const b = new Uint8Array(this.wasmInstance.memory.buffer.slice(p, p + maxLength))
    while(b[len] !== 0) {
      len++
      if (len > maxLength) break
    }
    return decoder.decode(new Uint8Array(b.buffer.slice(0, len)))
  }

  process(inputs, outputs, parameters) {
    if (!this?.wasmInstance?.process) return true;
    const output = outputs[0];
    for (let channel = 0; channel < output.length; channel++) {
      const outputChannel = output[channel];
      for (let i = 0; i < outputChannel.length; i++) {
        outputChannel[i] = this.wasmInstance.process(this.position++, 0, channel);
      }
    }
    return true;
  }

  // TODO: allow getting/setting params
}

registerProcessor('null-unit', NullUnitProcessor);
