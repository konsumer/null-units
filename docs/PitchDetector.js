// this is a pseudo-unit that detects closes note
// this is not super-accurate for lower frequencies

export default class PitchDetector {
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

  connect(source) {
    source.connect(this.analyser)
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

  start() {
    this.isRunning = true
    this.update()
  }

  // Main update function
  update() {
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
