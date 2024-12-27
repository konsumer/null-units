// this is a pitch-detector unit

class PitchDetector {
  constructor(audioContext) {
    this.audioContext = audioContext
    this.analyser = this.audioContext.createAnalyser()
    this.analyser.fftSize = 4096
    this.analyser.smoothingTimeConstant = 0.2
    this.analyser.minDecibels = -100
    this.analyser.maxDecibels = -30
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

  getFrequency() {
      const floatArray = new Float32Array(this.analyser.frequencyBinCount);
      this.analyser.getFloatFrequencyData(floatArray);

      const nyquist = this.audioContext.sampleRate / 2;
      const binWidth = nyquist / floatArray.length;

      let maxAmplitude = -Infinity;
      let peakBin = -1;

      const minBin = Math.floor(15 / binWidth);

      for (let i = minBin; i < floatArray.length; i++) {
          // This is the line that changes:
          const amplitude = (floatArray[i-1] + floatArray[i] * 3 + floatArray[i+1]) / 5;

          if (amplitude > maxAmplitude && amplitude > -80) {
              maxAmplitude = amplitude;
              peakBin = i;
          }
      }

      return peakBin > 0 ? peakBin * binWidth : 0;
  }

  // Convert frequency to nearest note
  getNote(frequency) {
    // A4 = 440hz
    const noteNumber = 12 * Math.log2(frequency / 440) // Use log2 directly
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

  // Main update function
  update() {
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
    if (this.onchange) {
      this.onchange(this.note)
    }
  }
}

export default class UnitPitch {
  constructor(manager) {
    this.manager = manager
    this.name = 'pitch'
    this.detector = new PitchDetector(manager.audioCtx)
    this.audioNode = this.detector.analyser
  }

  // load unit, and return info about it
  load() {
    this.loaded = true
  }

  // unload unit
  unload() {}

  // set a param
  set_param(paramId, value, timefromNowInSeconds=0) {
    // no params
  }

  // get a param
  get_param(paramId) {
    // no params
  }

  update() {
    if (this.html) {
      const pre = this.html.querySelector('pre')
      if (pre) {
        this.detector.update()
        pre.innerHTML = JSON.stringify(this.detector.note, null, 2)
      }
    }
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
    fs.appendChild(document.createElement('pre'))

    this.html = f
    return this.html
  }
}
