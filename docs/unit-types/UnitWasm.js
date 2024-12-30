// this is a wasm-based unit

import genUI from '../gen-ui.js'

// enum from units
const NULL_PARAM_BOOL = 0
const NULL_PARAM_I32 = 1
const NULL_PARAM_F32 = 2

export default class UnitWasm {
  constructor(manager, name, dirUnits) {
    this.manager = manager
    this.dirUnits = dirUnits
    this.params = []
    this.name = name
  }

  // load unit, and return info about it
  async load() {
    this.audioNode = new AudioWorkletNode(this.manager.audioCtx, 'null-unit')
    const bytes = await fetch(`${this.dirUnits}/${this.name}.wasm`).then(r => r.arrayBuffer())

    // send samples to unit
    for (const index in this.manager.samples) {
      this.audioNode.port.postMessage({ type: 'set_data', sample: this.manager.samples[index], index })
    }

    // wait for info
    // TODO: this could be more robust, like a promise-based send/retval thing
    await new Promise((resolve, reject) => {
      this.audioNode.port.onmessage = async e => {
        if (e?.data?.type === 'info' && e.data.id === this.id) {
          this.name = e.data.info.name
          this.channelsIn = e.data.info.channelsIn
          this.channelsOut = e.data.info.channelsOut
          this.params = e.data.info.params
          resolve()
        }
      }
      this.audioNode.port.postMessage({ type: 'load', bytes, id: this.id })
    })

    this.loaded = true
  }

  // unload unit
  unload() {}

  // set a param
  async set_param(paramId, value, timefromNowInSeconds=0) {
    const p = Number.isFinite(paramId) ? paramMap[paramId] : paramId
    const pi = this.params.findIndex(pd => pd.name === p)

    if (pi === -1) {
      return
    }

    if (this.params[pi]) {
      this.params[pi].value = value

      // TODO: float inputs still jump badly on input
      if (this.params[pi].input) {
        if (this.params[pi].type === NULL_PARAM_BOOL && this.params[pi].input.checked != !!value) {
          this.params[pi].input.checked = !!value
          this.params[pi].input.dispatchEvent(new Event('change'))
        }
        if (this.params[pi].type === NULL_PARAM_I32 && parseInt(this.params[pi].input.value) !== parseInt(value)) {
          this.params[pi].input.value = parseInt(value).toString()
          this.params[pi].input.dispatchEvent(new Event('change'))
        }
        if (this.params[pi].type === NULL_PARAM_F32 && parseFloat(this.params[pi].input.value).toFixed(4) !== parseFloat(value).toFixed(4)) {
          this.params[pi].input.value = parseFloat(value).toFixed(4)
          this.params[pi].input.dispatchEvent(new Event('change'))
        }
      }
    }

    setTimeout(() => {
      this.audioNode.port.postMessage({ id: this.id, type: 'param_set', paramID: pi, value })
    }, timefromNowInSeconds * 1000)
  }

  // get a param
  async get_param(paramId) {}

  // web only: generate a UI
  ui() {
    this.html = genUI(this)
    return this.html
  }
}
