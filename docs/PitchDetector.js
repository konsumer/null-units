// this is a pseudo-unit that detects closes note

export default class PitchDetector {
  constructor (audioContext) {
    this.audioContext = audioContext
    this.analyser = this.audioContext.createAnalyser()
    this.analyser.fftSize = 2048
    this.bufferLength = this.analyser.frequencyBinCount
    this.dataArray = new Float32Array(this.bufferLength)

    // Note frequencies (A4 = 440hz as reference)
    this.noteStrings = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

    this.note = {
      note: 'UNKOWN',
      octave: 0,
      frequency: 0,
      midiNote: 0
    }
    this.isRunning = false
  }

  connect (source) {
    source.connect(this.analyser)
  }

  // Get frequency from autocorrelation
  getFrequency () {
    this.analyser.getFloatTimeDomainData(this.dataArray)

    const ACF = [] // autocorrelation function
    let sum = 0

    // Calculate autocorrelation
    for (let lag = 0; lag < this.bufferLength; lag++) {
      let tmp = 0
      for (let i = 0; i < this.bufferLength - lag; i++) {
        tmp += this.dataArray[i] * this.dataArray[i + lag]
      }
      ACF[lag] = tmp
      sum += tmp
    }

    // Find first peak after zero
    let foundPeak = false
    let peak = -1

    for (let i = 1; i < this.bufferLength; i++) {
      if (ACF[i] > ACF[i - 1] && ACF[i] > ACF[i + 1]) {
        if (!foundPeak && ACF[i] > 0) {
          foundPeak = true
          peak = i
          break
        }
      }
    }

    if (peak !== -1) {
      const frequency = this.audioContext.sampleRate / peak
      return frequency
    }

    return 0
  }

  // Convert frequency to nearest note
  getNote (frequency) {
    // A4 = 440hz
    const noteNumber = 12 * (Math.log(frequency / 440) / Math.log(2))
    const note = Math.round(noteNumber) + 69 // MIDI note number
    const octave = Math.floor((note - 12) / 12)
    const noteName = this.noteStrings[note % 12]

    return {
      note: noteName,
      octave,
      frequency,
      midiNote: note
    }
  }

  start () {
    this.isRunning = true
    this.update()
  }

  // Main update function
  update () {
    if (this.isRunning) {
      const frequency = this.getFrequency()
      if (frequency > 0) {
        this.note = this.getNote(frequency)
      } else {
        this.note = {
          note: 'UNKOWN',
          octave: 0,
          frequency: 0,
          midiNote: 0
        }
      }
    }
    if (this.onchange) {
      this.onchange(this.note)
    }
    requestAnimationFrame(() => this.update())
  }
}
