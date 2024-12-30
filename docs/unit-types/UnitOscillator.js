// this is an oscillator for testing

import genUI from '../gen-ui.js'

// this numeric lookip allows params that look similar to othe untits
const paramMap = {
  0: 'type',
  1: 'note'
}

export default class UnitOscillator {
  constructor(manager) {
    this.manager = manager
    this.name = 'osc'
    this.params = [
      { "type": 1, "min": 0, "max": 3, "value": 0, "name": "type"},
      { "type": 2, "min": 0, "max": 127, "value": 0, "name": "note" }
    ]
    this.audioNode = this.manager.audioCtx.createOscillator()
    this.audioNode.type = 'sine'
    this.audioNode.frequency.setValueAtTime(0, 0)
  }

  // load unit, and get info about it
  load() {
    this.audioNode.start()
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

    if (p === 'type') {
      // sin/sqr/tri/saw
      if (Math.floor(value) === 0) {
        this.audioNode.type = 'sine'
      }
      if (Math.floor(value) === 1) {
        this.audioNode.type = 'square'
      }
      if (Math.floor(value) === 2) {
        this.audioNode.type = 'triangle'
      }
      if (Math.floor(value) === 3) {
        this.audioNode.type = 'sawtooth'
      }
    }
    if (p === 'note') {
      this.audioNode.frequency.setValueAtTime(440 * Math.pow(2, (value - 69) / 12), timefromNowInSeconds)
    }

    if (this.params[pi]) {
      this.params[pi].value = value

      if (this.params[pi].input){
        if (this.params[pi].input.value != value) {
          this.params[pi].input.value = value
          this.params[pi].input.dispatchEvent(new Event('change'))
        }
      }
    }
  }

  // get a param
  async get_param(paramId) {
    const p = Number.isFinite(paramId) ? paramMap[paramId] : paramId
    const pi = this.params.findIndex(pd => pd.name === p)

    if (pi === -1) {
      return
    }

    return this.params[pi].value
  }

  // web only: generate a UI
  ui() {
    this.html = genUI(this)
    return this.html
  }
}
