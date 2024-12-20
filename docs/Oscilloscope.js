// this is a pseudo-unit that makes an aoscilloscope
export default class Oscilloscope {
  constructor(audioContext) {
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

  set canvas(element) {
    if (!element) return
    this.canvasElement = element
    this.ctx = element.getContext('2d')
    // Set default dimensions if not specified
    if (!element.width) element.width = 600
    if (!element.height) element.height = 200
  }

  get canvas() {
    return this.canvasElement
  }

  get destination() {
    return this.analyser
  }

  start() {
    if (!this.canvas || !this.ctx) {
      throw new Error('Canvas must be set before starting oscilloscope')
    }
    this.isRunning = true
    this.draw()
  }

  stop() {
    this.isRunning = false
  }

  draw() {
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
