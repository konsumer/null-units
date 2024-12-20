// generate wave-forms that wavetable can load (float32)
// type is 0/1/2/3 sin/sqr/tri/saw
export default function waveGenerator(type = 0, size = 256) {
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
