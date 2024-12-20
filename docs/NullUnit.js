// this represets a single instance of the wasm unit

// this acts like an audio-context, but with a scope to look at
export class Oscilloscope {
  constructor (audioContext) {
    this.audioContext = audioContext
    this.canvas = null
    this.ctx = null
    this.analyser = audioContext.createAnalyser()
    this.analyser.fftSize = 2048
    this.bufferLength = this.analyser.frequencyBinCount
    this.dataArray = new Float32Array(this.bufferLength)
    this.isRunning = false

    // Visual settings
    this.backgroundColor = 'rgb(255, 255, 255)'
    this.lineColor = 'rgb(255, 0, 0)'
    this.zeroColor = 'rgb(200, 200, 200)'
    this.lineWidth = 2
  }

  set canvas (element) {
    if (!element) return
    this.canvasElement = element
    this.ctx = element.getContext('2d')
    // Set default dimensions if not specified
    if (!element.width) element.width = 600
    if (!element.height) element.height = 200
  }

  get canvas () {
    return this.canvasElement
  }

  get destination () {
    return this.analyser
  }

  start () {
    if (!this.canvas || !this.ctx) {
      throw new Error('Canvas must be set before starting oscilloscope')
    }
    this.isRunning = true
    this.draw()
  }

  stop () {
    this.isRunning = false
  }

  draw () {
    if (!this.isRunning) return

    const { ctx, canvas, dataArray, bufferLength, analyser } = this

    // Request next animation frame
    requestAnimationFrame(() => this.draw())

    // Get waveform data
    analyser.getFloatTimeDomainData(dataArray)

    // Clear canvas
    ctx.fillStyle = this.backgroundColor
    ctx.fillRect(0, 0, canvas.width, canvas.height)

    // Draw zero line
    ctx.lineWidth = this.lineWidth
    ctx.strokeStyle = this.zeroColor
    ctx.beginPath()
    ctx.moveTo(0, canvas.height / 2)
    ctx.lineTo(canvas.width, canvas.height / 2)
    ctx.stroke()

    // Draw waveform
    ctx.beginPath()
    ctx.lineWidth = this.lineWidth
    ctx.strokeStyle = this.lineColor

    const sliceWidth = canvas.width / bufferLength
    let x = 0

    for (let i = 0; i < bufferLength; i++) {
      const v = dataArray[i]
      const y = ((v + 1) * canvas.height) / 2

      if (i === 0) {
        ctx.moveTo(x, y)
      } else {
        ctx.lineTo(x, y)
      }

      x += sliceWidth
    }

    ctx.stroke()
  }
}

export const dir = import.meta.url.split('/').slice(0, -1).join('/')

// extend AudioContext to add NullUnit stuff
export default class NullUnitContext extends AudioContext {
  createOscilloscope () {
    return new Oscilloscope(this)
  }

  async createUnit (url) {
    const bytes = await fetch(url).then(r => r.arrayBuffer())
    await this.audioWorklet.addModule(`${dir}/unit-processor.js`)
    const unit = new AudioWorkletNode(this, 'null-unit')
    let info = {}
    let paramNames = []

    // wait for initial info
    await new Promise((resolve, reject) => {
      unit.port.onmessage = ({ data: { type, ...args } }) => {
        if (type === 'info') {
          info = args.info
          paramNames = info.params.map(p => p.name)
          resolve()
        }
      }
      unit.port.postMessage({ type: 'load', bytes })
    })

    const handler = {
      get (...a) {
        if (a[1] === 'connect') {
          return outNode => unit.connect(outNode)
        }
        const i = paramNames.indexOf(a[1])
        if (i > -1) {
          const param = info.params[i]
          return {
            setValueAtTime (value, time) {
              setTimeout(() => unit.port.postMessage({ type: 'param_set', paramID: i, value }), time / 1000)
            },

            // this is just the initial value
            // but I could periodically update to grab it
            value: param.value
          }
        }
        return Reflect.get(...a)
      }
    }
    return new Proxy({}, handler)
  }
}
