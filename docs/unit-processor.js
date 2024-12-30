// this is the actual audio worklet that runs the wasm for units

import EasyWasiLite from './EasywasiLite.js'

// enum from units
const NULL_PARAM_BOOL = 0
const NULL_PARAM_I32 = 1
const NULL_PARAM_F32 = 2

// fully loads the wasm
async function setupWasm(bytes, wrapper) {
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
        console.log(wasi_snapshot_preview1.getString(msgPtr))
      },

      get_data_floats (id, offset, length, out) {
        if (!wrapper.data[id]?.buffer) {
          return
        }
        const mem = new Uint8Array(wasm.memory.buffer)
        mem.set(new Uint8Array(wrapper.data[id].buffer.slice(offset, offset + (length * 4))), out)
      }
    }
  }
  const wasm = { ...(await WebAssembly.instantiate(bytes, importObject)).instance.exports, memory, wasi: wasi_snapshot_preview1 }
  wasi_snapshot_preview1.start(wasm)
  return wasm
}

// get info about single param (for get_info)
function get_param_value (rawValue, type) {
    const view = new DataView(new ArrayBuffer(4))
    view.setUint32(0, rawValue, true)
    switch (type) {
      case NULL_PARAM_BOOL:
        return Boolean(rawValue)
      case NULL_PARAM_I32:
        return view.getInt32(0, true)
      case NULL_PARAM_F32:
        return view.getFloat32(0, true)
      default:
        throw new Error(`Unknown parameter type: ${type}`)
    }
  }

// get info (and parse it) about plugin from wasm
function get_info(wasm) {
  const info = {}
  const p = wasm.get_info()
  const iview = new DataView(wasm.memory.buffer.slice(p, p + 12))
  info.name = wasm.wasi.readString(iview.getUint32(0, true))
  info.channelsIn = iview.getUint8(4, true)
  info.channelsOut = iview.getUint8(5, true)
  const paramCount = iview.getUint8(6, true)
  const paramsPtr = iview.getUint32(8, true)
  info.params = []
  for (let i = 0; i < paramCount; i++) {
    const paramOffset = paramsPtr + (i * 20)
    const paramView = new DataView(wasm.memory.buffer.slice(paramOffset, paramOffset + 20))
    const param = {
      type: paramView.getUint32(0, true),
      min: get_param_value(paramView.getUint32(4, true), paramView.getUint32(0, true)),
      max: get_param_value(paramView.getUint32(8, true), paramView.getUint32(0, true)),
      value: get_param_value(paramView.getUint32(12, true), paramView.getUint32(0, true)),
      name: wasm.wasi.readString(paramView.getUint32(16, true))
    }
    info.params.push(param)
  }
  return info
}

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
        case 'load':
          this.wasm = await setupWasm(args.bytes, this)
          this.paramPtr = this.wasm.malloc(4)
          this.info =  get_info(this.wasm)
          // send the response with the same id
          this.port.postMessage({ type: 'info', id: args.id, info: this.info })
          break
        case 'set_data':
          const { index, sample } = args
          this.data[index] = sample
          break
        case 'param_set':
          const { id, paramID, value } = args
          this.info.params[paramID].value = value
          const view = new DataView(this.wasm.memory.buffer, this.paramPtr, 4)
          if (this.info.params[paramID].type == NULL_PARAM_F32) {
            view.setFloat32(0, value, true)
          }
          if (this.info.params[paramID].type == NULL_PARAM_I32) {
            view.setInt32(0, value, true)
          }
          if (this.info.params[paramID].type == NULL_PARAM_BOOL) {
            view.setInt32(0, value ? 1 : 0, true)
          }
          this.wasm.param_set(paramID, this.paramPtr)
          break
        default:
          console.log('unhandled message', { type, args })
      }
    }
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
  NULL_PARAM_BOOL,  // stored as int32_t
  NULL_PARAM_I32,
  NULL_PARAM_F32
};

union NullUnitParamValue (32) {
  int32_t i;
  float f;
};
*/

registerProcessor('null-unit', NullUnitProcessor)
