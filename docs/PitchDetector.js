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
  getFrequency() {
      this.analyser.getFloatTimeDomainData(this.dataArray);

      const ACF = [];

      // Calculate autocorrelation
      for (let lag = 0; lag < this.bufferLength; lag++) {
          let tmp = 0;
          for (let i = 0; i < this.bufferLength - lag; i++) {
              tmp += this.dataArray[i] * this.dataArray[i + lag];
          }
          ACF[lag] = tmp;
      }

      // Normalize ACF
      const maxACF = Math.max(...ACF);
      for (let i = 0; i < this.bufferLength; i++) {
          ACF[i] = ACF[i] / maxACF;
      }

      // Set frequency range limits for MIDI notes
      const minFreq = 80;   // Around MIDI note 40 (E2)
      const maxFreq = 1000; // Around MIDI note 83 (B5)
      const minLag = Math.floor(this.audioContext.sampleRate / maxFreq);
      const maxLag = Math.floor(this.audioContext.sampleRate / minFreq);

      const threshold = 0.3;  // Lower threshold for peak detection

      let bestPeak = -1;
      let bestPeakValue = -1;

      // Find the first strong peak after the first zero crossing
      let foundZeroCrossing = false;

      for (let i = minLag; i < maxLag; i++) {
          if (!foundZeroCrossing && ACF[i] <= 0) {
              foundZeroCrossing = true;
              continue;
          }

          if (foundZeroCrossing &&
              ACF[i] > threshold &&
              ACF[i] > ACF[i - 1] &&
              ACF[i] > ACF[i + 1]) {
              // Verify this is a genuine peak by checking surrounding values
              const isPeakValid = ACF[i] > ACF[i - 2] &&
                                ACF[i] > ACF[i + 2] &&
                                ACF[i] > ACF[i - 3] &&
                                ACF[i] > ACF[i + 3];

              if (isPeakValid && ACF[i] > bestPeakValue) {
                  bestPeak = i;
                  bestPeakValue = ACF[i];
                  break; // Take the first valid peak
              }
          }
      }

      if (bestPeak !== -1) {
          const frequency = this.audioContext.sampleRate / bestPeak;
          return frequency;
      }

      return 0;
  }

  // Convert frequency to nearest note
  getNote (frequency) {
      // A4 = 440hz
      const noteNumber = 12 * (Math.log2(frequency / 440)) // Use log2 directly
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
