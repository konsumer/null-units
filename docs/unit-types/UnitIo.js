// this represents audio IO of system

export default class UnitIo {
  constructor(manager, dirWorker) {
    this.manager = manager
    this.dirWorker = dirWorker
    this.audioNode = manager.audioCtx.destination
  }

  // load unit, and return info about it
  async load() {
    // setup wasm unit-processor
    await this.manager.audioCtx.audioWorklet.addModule(`${this.dirWorker}/unit-processor.js`)
    this.loaded = true
  }

  // unload unit
  unload() {
    // nothing to do here
  }

  // set a param
  set_param(paramId, value, timefromNowInSeconds=0) {
    // no params
  }

  // get a param
  async get_param(paramId) {
    // no params
  }
}
