import EasyWasiLite from './EasywasiLite.js'

const getParamVal = (type, paramView, offset=0) => {
  switch (type) {
    case 0: // NULL_PARAM_BOOL
      return Boolean(paramView.getInt8(offset));

    case 1: // NULL_PARAM_U8
      return paramView.getUint8(offset);

    case 2: // NULL_PARAM_I8
      return paramView.getInt8(offset);

    case 3: // NULL_PARAM_U16
      return paramView.getUint16(offset, true);

    case 4: // NULL_PARAM_I16
      return paramView.getInt16(offset, true);

    case 5: // NULL_PARAM_U32
      return paramView.getUint32(offset, true);

    case 6: // NULL_PARAM_I32
      return paramView.getInt32(offset, true);

    case 7: // NULL_PARAM_F32
      return paramView.getFloat32(offset, true);

    case 8: // NULL_PARAM_U64
      return Number(paramView.getBigUint64(offset, true));

    case 9: // NULL_PARAM_I64
      return Number(paramView.getBigInt64(offset, true));

    case 10: // NULL_PARAM_F64
      return paramView.getFloat64(offset, true);
    default:
      console.warn(`Unknown parameter type: ${type}`);
      return null;
  }
};

const setParamVal = (type, value, paramView, offset = 0) => {
  switch (type) {
    case 0: // NULL_PARAM_BOOL
      return paramView.setInt8(offset, value ? 0 : 1);

    case 1: // NULL_PARAM_U8
      return paramView.setUint8(offset, value);

    case 2: // NULL_PARAM_I8
      return paramView.setInt8(offset, value);

    case 3: // NULL_PARAM_U16
      return paramView.setUint16(offset, value, true);

    case 4: // NULL_PARAM_I16
      return paramView.setInt16(offset, value, true);

    case 5: // NULL_PARAM_U32
      return paramView.setUint32(offset, value, true);

    case 6: // NULL_PARAM_I32
      return paramView.setInt32(offset, value, true);

    case 7: // NULL_PARAM_F32
      return paramView.setFloat32(offset, value, true);

    case 8: // NULL_PARAM_U64
      return paramView.setBigUint64(offset, BigInt(value), true);

    case 9: // NULL_PARAM_I64
      return Number(paramView.setBigInt64(offset, BigInt(value), true));

    case 10: // NULL_PARAM_F64
      return paramView.setFloat64(offset, value, true);
    default:
      console.warn(`Unknown parameter type: ${type}`);
      return null;
  }
}

// this manages the actual wasm for a unit
// and will be loaded inside audio-worklet
export class NullUnitWasm {
  constructor() {
    // this holds samples and things]
    this.data = {}

    // this will have info about the unit, ater you run load()
    this.info = {}
  }

  // load the wasm
  async load(wasmBytes) {
    // this is host-functions exposed to the unit-wasm
    const wasi_snapshot_preview1 = new EasyWasiLite()

    const importObject = {
      env: {
        trace(msgPtr) {
          console.log(this.getString(msgPtr))
        },

        get_data_floats(id, offset, length, out) {
          const mem = new Uint8Array(this.wasm.memory.buffer)
          mem.set(new Uint8Array(this.data[id].buffer.slice(offset, offset + length * 4)), out)
        }
      },
      wasi_snapshot_preview1
    }

    // bind imports to this
    for (const m of Object.keys(importObject)) {
      for (const f of Object.keys(importObject[m])) {
        if (typeof importObject[m][f] === 'function') {
          importObject[m][f] = importObject[m][f].bind(this)
        }
      }
    }

    // load wasm
    this.wasm = (await WebAssembly.instantiate(wasmBytes, importObject)).instance.exports
    wasi_snapshot_preview1.start(this.wasm)
    this.wasi_snapshot_preview1 = wasi_snapshot_preview1

    // this is reused to set param-values
    this.paramPtr = this.wasm.malloc(8)
    this.paramReturnView = new DataView(this.wasm.memory.buffer, this.paramPtr, 8);

    return this.update_info()
  }

  // get info about unit
  update_info() {
    if (!this?.wasm?.get_info) {
      return undefined;
    }
    const ptr = this.wasm.get_info();
    const info = new DataView(this.wasm.memory.buffer.slice(ptr, ptr+12))
    const out = {
      name: this.wasi_snapshot_preview1.readString(info.getUint32(0, true)),
      channelsIn: info.getUint8(4),
      channelsOut: info.getUint8(5),
      params: []
    }
    const paramCount = info.getUint8(6)
    const paramsPtr = info.getUint32(8, true)

    for (let i = 0; i < paramCount; i++) {
      const paramPtr = paramsPtr + (i * 40);
      const paramView = new DataView(this.wasm.memory.buffer, paramPtr, 40);
      const type = paramView.getInt32(0, true)
      out.params.push({
        type,
        min: getParamVal(type, paramView, 8),
        max: getParamVal(type, paramView, 16),
        value: getParamVal(type, paramView, 24),
        name: this.wasi_snapshot_preview1.readString(paramView.getUint32(32, true))
      })
    }
    this.info = out
    return out
  }

  param_set(paramID, value) {
    setParamVal(this.info.params[paramID].type, value, this.paramReturnView)
  }

  param_get(paramID) {
    return getParamVal(this.info.params[paramID].type, this.paramReturnView)
  }

  process(position, input, channel) {
    return this.wasm.process(position, input, channel)
  }
}

class NullUnitProcessor extends AudioWorkletProcessor {
  constructor() {
    // TODO: handle these from wasm
    super({numberOfInputs: 1, numberOfOutputs: 1})

    this.position = 0

    this.port.onmessage = async ({ data: { type, ...args} }) => {
      switch (type) {
        case 'load':
          this.unit = new NullUnitWasm()
          this.info = await this.unit.load(args.bytes)
          this.port.postMessage({ type: 'info', info: this.info })
          break
        case 'param_set':
          if (this.unit) {
            this.unit.param_set(args.paramID, args.value)
          }
          break
        case 'param_get':
          if (this.unit) {
            this.port.postMessage({ type: 'param_get', value: this.unit.param_get(args.paramID), paramID: args.paramID })
          }
          break
      }
    }
  }

  process(inputs, outputs, parameters) {
    if (this?.unit?.process) {
      const output = outputs[0]
      const input = inputs[0]
      for (let channel = 0; channel < output.length; channel++) {
        const outputChannel = output[channel]
        const inputChannel = input[channel] || []
        for (let i = 0; i < outputChannel.length; i++) {
          outputChannel[i] = this.unit.process(this.position++, inputChannel[i] || 0, channel)
        }
      }
    }
    return true
  }
}

registerProcessor("null-unit", NullUnitProcessor);
