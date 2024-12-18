// this is a wrapper class for use in js

// get location of other files based in this file's location
function getFilePath() {
  const u = import.meta.url.split('/')
  u.pop()
  return u.join('/')
}

export default class NullUnit {
  constructor(context) {
    this.context = context
    this.messageMap = new Map()
    this.messageId = 0
    this.info = {}
  }

  // async send that can await return
  async send(message) {
    const id = this.messageId++
    return new Promise((resolve) => {
      this.messageMap.set(id, { resolve })
      this.audioWorkletNode.port.postMessage({ id, message })
    })
  }

  // load the wasm null-unit
  async load(name) {
    const dir = getFilePath()
    console.log(dir)

    await this.context.audioWorklet.addModule(`${dir}/unit-processor.js`)
    this.audioWorkletNode = new AudioWorkletNode(this.context, 'null-unit')
    // setup async messaging
    this.audioWorkletNode.port.onmessage = (event) => {
      const { id, response } = event.data
      if (this.messageMap.has(id)) {
        const { resolve } = this.messageMap.get(id)
        resolve(response)
        this.messageMap.delete(id)
      }
    }

    const bytes = await fetch(`${dir}/${name}.wasm`).then((r) => r.arrayBuffer())
    this.info = await this.send({ type: 'load', bytes })
  }

  // set a unit param
  async setParam(name, value) {
    if (!this?.info?.param_names) {
      console.error('unit not loaded')
      return
    }
    const param = this.info.param_names.indexOf(name)
    if (param === -1) {
      console.error(`param '${name}' not found.`)
      return
    }
    return this.send({ type: 'param_set', param, value })
  }

  // get a loaded sample
  getSample(index) {
    return this.send({ type: 'get_sample', index })
  }

  // set an existing sample (data is float32)
  setSample(index, data) {
    return this.send({ type: 'set_sample', index, data })
  }

  // add a new sample (data is float32)
  async addSample(data) {
    const lastSample = (await this.send({ type: 'get_last_sample' })) | 0
    return this.setSample(lastSample + 1, data)
  }

  connect(outlet) {
    this.audioWorkletNode.connect(outlet)
  }

  // clean up
  disconnect() {
    this.audioWorkletNode.disconnect()
    this.messageMap.clear()
  }
}
