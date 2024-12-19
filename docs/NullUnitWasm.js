// this represets a single instance of the wasm unit

import EasyWasiLite from './EasyWasiLite.js'

export default class NullUnitWasm {
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
    this.update_info()

    return this.info
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
      const getVal = (offset) => {
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

      out.params.push({
        type,
        min: getVal(8),
        max: getVal(16),
        value: getVal(24),
        name: this.wasi_snapshot_preview1.readString(paramView.getUint32(32, true))
      })
    }
    this.info = out
    return out
  }
}
