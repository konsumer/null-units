// this is what I used to generate my simple wavetable samples

import { writeFile } from 'fs/promises'

function waveGenerator (type = 0, size = 256) {
  const out = new Float32Array(size)

  for (let i = 0; i < size; i++) {
    const x = (i / size) * 2 * Math.PI // normalize to 0-2π

    switch (type) {
      case 1: // sqr
        out[i] = x < Math.PI ? 1.0 : -1.0
        break

      case 2: // tri
        out[i] = 1 - 4 * Math.abs(Math.round(x / (2 * Math.PI)) - x / (2 * Math.PI))
        break

      case 3: // saw
        out[i] = 1 - (2 * (x % (2 * Math.PI))) / (2 * Math.PI)
        break

      default: // sin
        out[i] = Math.sin(x)
    }
  }

  return out
}

await Promise.all([
  writeFile('samples/sin.raw', new Uint8Array(waveGenerator(0).buffer)),
  writeFile('samples/sqr.raw', new Uint8Array(waveGenerator(1).buffer)),
  writeFile('samples/tri.raw', new Uint8Array(waveGenerator(2).buffer)),
  writeFile('samples/saw.raw', new Uint8Array(waveGenerator(3).buffer)),
])
