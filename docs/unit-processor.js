// this is the actual audio worklet that runs the wasm for units

import EasyWasiLite from './EasywasiLite.js'

class NullUnitProcessor extends AudioWorkletProcessor {
  constructor () {
    // max-number of inputs/outputs
    super({ numberOfInputs: 8, numberOfOutputs: 8 })

    this.position = 0
    this.unitID = 0
    this.info = {}
    this.data = {}

    this.port.onmessage = async ({ data: { type, ...args } }) => {
      switch (type) {
        // load a wasm-unit
        case 'load':
          this.unitID = args.id
          const info = await this.loadWasm(args.bytes)

          // this is used to pas params to wasm
          this.paramPtr = this.wasm.malloc(4)

          this.info = info
          this.port.postMessage({ type: 'info', id: args.id, info })
          break

        // set a sample, by id number
        case 'set_data':
          const { index, sample } = args
          this.data[index] = sample
          break

        // set a param
        case 'param_set':
          const { id, paramID, value } = args
          const param = this.info.params[paramID]
          const view = new DataView(this.wasm.memory.buffer)
          view.setUint32(this.paramPtr, this.setParamValue(value, param.type), true)
          this.wasm.param_set(paramID, this.paramPtr)
          this.info.params[paramID].value = value
          break

        // get a a param
        case 'param_get':
          break
      }
    }
  }

  // this will setup wasm unit & return info
  async loadWasm (bytes) {
    const wasi_snapshot_preview1 = new EasyWasiLite()

    const memory = new WebAssembly.Memory({
      initial: 2,
      maximum: 30
    })

    const importObject = {
      wasi_snapshot_preview1,
      env: {
        memory,

        trace (msgPtr) {
          console.log(this.wasi.getString(msgPtr))
        },

        get_data_floats (id, offset, length, out) {
          if (!this.data[id]?.buffer) {
            return
          }
          const mem = new Uint8Array(this.wasm.memory.buffer)
          mem.set(new Uint8Array(this.data[id].buffer.slice(offset, offset + (length * 4))), out)
        }
      }
    }

    // bind importObject to myself
    for (const m of Object.keys(importObject)) {
      for (const f of Object.keys(importObject[m])) {
        if (typeof importObject[m][f] === 'function') {
          importObject[m][f] = importObject[m][f].bind(this)
        }
      }
    }

    this.wasm = { ...(await WebAssembly.instantiate(bytes, importObject)).instance.exports, memory }
    wasi_snapshot_preview1.start(this.wasm)
    this.wasi = wasi_snapshot_preview1
    return this.getInfo()
  }

  setParamValue (value, type) {
    const view = new DataView(new ArrayBuffer(4))
    switch (type) {
      case 0: // NULL_PARAM_BOOL
        return Number(Boolean(value))
      case 1: // NULL_PARAM_U32
        return value >>> 0
      case 2: // NULL_PARAM_I32
        view.setInt32(0, value, true)
        return view.getUint32(0, true)
      case 3: // NULL_PARAM_F32
        view.setFloat32(0, value, true)
        return view.getUint32(0, true)
      default:
        throw new Error(`Unknown parameter type: ${type}`)
    }
  }

  // Helper function to interpret the param-value based on type
  getParamValue (rawValue, type) {
    const view = new DataView(new ArrayBuffer(4))
    view.setUint32(0, rawValue, true)
    switch (type) {
      case 0: // NULL_PARAM_BOOL
        return Boolean(rawValue)
      case 1: // NULL_PARAM_U32
        return rawValue
      case 2: // NULL_PARAM_I32
        return view.getInt32(0, true)
      case 3: // NULL_PARAM_F32
        return view.getFloat32(0, true)
      default:
        throw new Error(`Unknown parameter type: ${type}`)
    }
  }

  // interrrogate unit for info, parse struct into JS object
  getInfo () {
    const info = {}
    const p = this.wasm.get_info()
    const iview = new DataView(this.wasm.memory.buffer.slice(p, p + 12))

    info.name = this.wasi.readString(iview.getUint32(0, true))
    info.channelsIn = iview.getUint8(4, true)
    info.channelsOut = iview.getUint8(5, true)
    const paramCount = iview.getUint8(6, true)
    const paramsPtr = iview.getUint32(8, true)

    info.params = []
    for (let i = 0; i < paramCount; i++) {
      const paramOffset = paramsPtr + (i * 20)
      const paramView = new DataView(this.wasm.memory.buffer.slice(paramOffset, paramOffset + 20))
      const param = {
        type: paramView.getUint32(0, true),
        min: this.getParamValue(paramView.getUint32(4, true), paramView.getUint32(0, true)),
        max: this.getParamValue(paramView.getUint32(8, true), paramView.getUint32(0, true)),
        value: this.getParamValue(paramView.getUint32(12, true), paramView.getUint32(0, true)),
        name: this.wasi.readString(paramView.getUint32(16, true))
      }
      info.params.push(param)
    }

    return info
  }

  process (inputs, outputs) {
    if (!this?.wasm?.process) return true

    const output = outputs[0]
    const input = inputs[0]

    for (let channel = 0; channel < output.length; channel++) {
      const outputChannel = output[channel]
      const inputChannel = input && input[channel] ? input[channel] : undefined

      for (let i = 0; i < outputChannel.length; i++) {
        let inputValue = 0
        if (inputChannel && typeof inputChannel[i] === 'number' && !isNaN(inputChannel[i])) {
          inputValue = inputChannel[i]
        }

        const processedValue = this.wasm.process(this.position++, inputValue, channel, sampleRate, currentTime)
        outputChannel[i] = isNaN(processedValue) ? 0 : processedValue
      }
    }
    return true
  }
}

/*
struct NullUnitnInfo (12) {
  char* name; // 4
  uint8_t channelsIn; // 1
  uint8_t channelsOut; // 1
  uint8_t paramCount; // 1
  // pad 1, for alignment
  NullUnitParamInfo* params; // 4
};

struct NullUnitParamInfo (20) {
  NullUnitParamType type; // 4
  NullUnitParamValue min; // 4
  NullUnitParamValue max; // 4
  NullUnitParamValue value; // 4
  char* name; // 4
};

enum NullUnitParamType (32) {
  NULL_PARAM_BOOL,  // stored as uint32_t
  NULL_PARAM_U32,
  NULL_PARAM_I32,
  NULL_PARAM_F32
};

union NullUnitParamValue (32) {
  uint32_t u;
  int32_t i;
  float f;
};
*/

registerProcessor('null-unit', NullUnitProcessor)
