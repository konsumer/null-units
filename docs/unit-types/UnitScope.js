// this numeric lookip allows params that look similar to othe untits
const paramMap = {
  0: 'backgroundColor',
  1: 'lineColor',
  2: 'zeroColor',
  3: 'lineWidth',
  4: 'canvas'
}

class Oscilloscope {
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

  draw () {
    const { ctx, canvas, dataArray, bufferLength, analyser } = this

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

// this is a an oscilloscope unit
export default class UnitScope {
  constructor(manager, canvas = document.createElement('canvas')) {
    this.manager = manager
    this.name = 'scope'
    this.params = []
    this.scope = new Oscilloscope(manager.audioCtx)
    this.scope.canvas = canvas
    this.audioNode = this.scope.analyser
  }

  // load unit, and return info about it
  async load() {
    this.loaded = true
  }

  // unload unit
  unload() {
    // nothing
  }

  // set a param
  async set_param(paramId, value, timefromNowInSeconds=0) {
    const p = Number.isFinite(paramId) ? paramMap[paramId] : paramId
    const pi = this.params.findIndex(pd => pd.name === p)

    if (pi === -1) {
      return
    }

    setTimeout(() => {
      this.params[pi].value = value
      this.scope[p] = value
    }, timefromNowInSeconds/1000)
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
    const f = document.createElement('form')
    f.id = `unit_${this.id}`
    const fs = document.createElement('fieldset')
    f.appendChild(fs)
    const l = document.createElement('legend')
    l.innerText = `${this.title || this.name} (${this.id})`
    fs.appendChild(l)
    fs.appendChild(this.scope.canvas)

    this.html = f
    return this.html
  }

  update() {
    this.scope.draw()
  }
}
