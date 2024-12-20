## Null Units

I am no expert at audio-programming, so I'm sure these could be improved (PRs welcome!)

Each of these effects is designed to provide classic studio-quality processing while remaining CPU-efficient and easy to use.

### Auto-Wah

Think Mu-Tron III, very popular in funk.

This auto-wah implementation features:

1. Envelope follower controls:
   - Sensitivity: How much the input affects the filter
   - Attack: How quickly the envelope responds to louder signals
   - Release: How quickly the envelope decays

2. Filter controls:
   - Resonance: Filter resonance/Q
   - Range: Frequency sweep range
   - Up/Down: Direction of frequency sweep

3. Features:
   - State variable filter for smooth response
   - Bidirectional sweep (up or down)
   - Envelope follower with adjustable response
   - Resonance compensation
   - Soft clipping protection

Usage tips:
- For classic funk: High sensitivity, medium attack/release, up direction
- For bass: Lower range, higher resonance, down direction
- For subtle movement: Lower sensitivity, longer attack/release
- For dramatic effects: High sensitivity, short attack, high resonance
- For rhythmic effects: Medium sensitivity, short attack/release

The effect uses a state variable filter which provides a smooth, musical response. The envelope follower can track the input signal's amplitude and control the filter frequency either upward (like a traditional auto-wah) or downward (for different tonal effects).

The resonance parameter adds emphasis around the sweep frequency, and the range parameter controls how wide the frequency sweep is. The up/down parameter lets you choose whether louder signals open or close the filter.


### Bitcrusher

For lo-fi digital effects.

This bitcrusher implementation features:

1. Three types of digital degradation:
   - Bit depth reduction (1-16 bits)
   - Sample rate reduction (1x to 100x)
   - Digital noise generation

2. Controls:
   - Bits: Reduces bit depth
   - Rate: Reduces sample rate
   - Noise: Adds digital noise
   - Mix: Wet/dry mix

3. Features:
   - Smooth parameter control
   - Signal-dependent noise
   - Sample & hold for rate reduction
   - Independent processing per channel

Usage tips:
- For subtle lo-fi: High bit depth (12-14 bits), slight rate reduction
- For 8-bit style: Set bits to 8, moderate rate reduction
- For extreme effects: Low bits (2-4), heavy rate reduction
- For digital artifacts: Add noise with low bit depth
- For "vintage digital": Moderate bits (8-12) with some noise

The effect can create various lo-fi sounds from subtle vintage digital warmth to extreme destruction. The noise parameter adds character that simulates the behavior of low-quality D/A converters.



### Chorus

Essential modulation effect for thickening sounds.

This chorus implementation features:

1. Multiple voices (up to 3)
2. Independent LFOs for each voice
3. Longer base delay times than flanger
4. No feedback path
5. Phase-offset LFOs for richer sound
6. Linear interpolation for smooth delay changes

The parameters are:
- Rate: Speed of the LFO (0.1-3 Hz)
- Depth: Amount of delay time modulation (0.0-1.0)
- Mix: Wet/dry mix (0.0-1.0)
- Voices: Number of chorus voices (1-3)

Key differences from flanger:
- Longer base delay times (20ms vs 1-10ms)
- Multiple voices with phase-offset LFOs
- No feedback path
- Generally slower modulation rates
- Wider modulation depths
- Focus on creating ensemble-like effects rather than swooshing

This creates the classic chorus effect heard on guitars, synthesizers, and vocals. It's great for:
- Thickening sounds
- Creating stereo width
- Adding movement to static sounds
- Creating ensemble-like effects

The multiple voices and phase-offset LFOs help create a rich, natural-sounding chorus effect that's less "metallic" than a flanger.


### Compressor

Very useful for guitar and general dynamics control.

This compressor implementation features:

1. Classic compressor controls:
   - Threshold: Level where compression starts
   - Ratio: Amount of compression
   - Attack: How quickly compression engages
   - Release: How quickly compression releases

2. Additional controls:
   - Knee: Smooth transition around threshold
   - Makeup: Output gain compensation
   - Mix: Parallel compression blend

3. Features:
   - Smooth envelope detection
   - Soft knee compression
   - Peak envelope tracking
   - dB-accurate calculations
   - Parallel compression option

Usage tips:
- For gentle compression: High threshold (-20dB), low ratio (2:1)
- For limiting: Low threshold (-3dB), high ratio (10:1+)
- For parallel compression: High ratio, full wet/dry mix control
- For "glue" compression: Medium threshold, low ratio, slow attack/release
- For punch: Fast attack, medium release, medium ratio

The compressor includes a soft knee for smooth transition around the threshold and proper envelope detection for musical response. The mix parameter allows for parallel compression techniques.

This compressor design aims to be clean and musical while still being CPU-efficient. The soft knee and proper envelope detection help it remain transparent when needed while still being capable of more aggressive compression when desired.

### copy

This is basically no-op, with no params. You can use it as a template.

### Delay

A digital delay effect with adjustable time, feedback, and stereo options. Features:
1. Adjustable delay time up to 2000ms
2. Feedback control for multiple echoes
3. Mix control for parallel processing
4. Tempo sync option for rhythmic delays
5. High-quality interpolation for smooth time changes

Great for:
- Creating space and depth
- Rhythmic echo effects
- Doubling sounds
- Ambient textures
- Ping-pong stereo effects



### Distortion

Could do both tube-style overdrive and harder distortion.

This distortion implementation features:

1. Multiple distortion types:
   - Soft clip (tube-like overdrive)
   - Hard clip (classic distortion)
   - Fuzz (asymmetric, aggressive)
   - Wavefolder (more experimental)
   - Bit crusher (digital distortion)

2. Controls:
   - Drive: Amount of distortion/gain
   - Tone: Tone control (low-pass filter)
   - Type: Distortion algorithm selection
   - Crunch: Adds harmonics and pre-gain
   - Mix: Wet/dry mix

3. Additional features:
   - DC offset removal
   - Tone control
   - Drive compensation
   - Harmonic enhancement
   - Output level compensation

The crunch parameter adds extra harmonics and pre-gain before the distortion stage, which can create more aggressive, "crunchy" tones. The different distortion types provide a wide range of tones from subtle warmth to extreme destruction.

Usage tips:
- For subtle overdrive: Use DIST_SOFT with low drive
- For classic distortion: Use DIST_HARD with medium drive
- For fuzz: Use DIST_FUZZ with high drive
- For digital destruction: Use DIST_CRUSH with high drive and crunch
- For experimental sounds: Use DIST_FOLD with various drive settings

The tone control helps shape the final sound, and the mix parameter allows for parallel distortion effects.

### Flanger

Similar to phaser but with shorter delay times.

This flanger implementation features:

1. Modulated delay line (shorter than regular delay)
2. LFO for delay time modulation
3. Adjustable rate (speed of flanging)
4. Adjustable depth (intensity of effect)
5. Base delay time control
6. Bipolar feedback control for different characters
7. Linear interpolation for smooth delay time changes

The parameters are:
- Rate: Speed of the LFO (0.1-10 Hz)
- Depth: Amount of delay time modulation (0.0-1.0)
- Delay: Base delay time (1-10 ms)
- Feedback: Amount and polarity of feedback (-0.95 to 0.95)

The flanger works by:
1. Creating a short delay line
2. Modulating the delay time with an LFO
3. Mixing the delayed signal with the dry signal
4. Adding feedback for resonance

The main difference from the phaser is that it uses actual delayed copies of the signal instead of phase shifting through all-pass filters. This tends to create a more "metallic" or "jet-like" sound compared to the phaser's more "swooshy" character.

Negative feedback values can create different tonal characters, and the base delay time can be adjusted to create various flanging effects from subtle doubling to more extreme metallic sounds.

## gain

This is a very simple amplifier. Yu can use it as a template for units with params.

### HPF (High-Pass Filter)
A clean high-pass filter for removing low frequencies. Features:
1. Adjustable cutoff frequency
2. Smooth frequency response
3. 12dB/octave slope
4. Stable at high resonance
5. Musical frequency range

Perfect for:
- Removing rumble and mud
- Creating space in a mix
- Sound design
- Thinning out sounds
- Contemporary bass drops

### LPF (Low-Pass Filter)

A classic low-pass filter for taming high frequencies.

Features:
1. Adjustable cutoff frequency
2. Smooth frequency response
3. 12dB/octave slope
4. Stable at high resonance
5. Wide frequency range

Ideal for:
- Removing harshness
- Warming up sounds
- Frequency sculpting
- Lo-fi effects
- Sound design basics

### MoogLPF

A Moog-style resonant low-pass filter emulation.

Features:
1. Classic ladder filter design
2. Resonance control with self-oscillation
3. Rich harmonic saturation
4. 24dB/octave slope
5. Characteristic analog warmth

Perfect for:
- Classic synthesizer sounds
- Rich bass filtering
- Acid-style sequences
- Warm sound shaping
- Resonant sweeps

### Phaser

Classic swooshing effect (like MXR Phase 90).

This phaser implementation features:

1. Four stage all-pass filter chain (like the Phase 90)
2. LFO modulation of the filter frequencies
3. Adjustable rate (speed of phasing)
4. Adjustable depth (intensity of effect)
5. Feedback control for resonance
6. Soft clipping to prevent overload

The parameters are:
- Rate: Speed of the LFO (0.1-10 Hz)
- Depth: Mix between dry and phased signal (0.0-1.0)
- Feedback: Amount of resonance (0.0-0.95)

The phaser works by:
1. Creating an LFO (sine wave)
2. Using the LFO to modulate all-pass filter frequencies
3. Running the signal through a chain of all-pass filters
4. Adding feedback for resonance
5. Mixing the processed signal with the dry signal

This creates the characteristic sweeping sound that's useful for everything from subtle modulation to deep, swooshing effects. The feedback parameter can create more dramatic resonant peaks, similar to how the Phase 90's internal trimpot works.

### Plate

A plate reverb simulation inspired by classic studio units.

Features:
1. Multiple parallel delay lines
2. Size/decay control
3. Damping for high-frequency absorption
4. Dense, smooth tail
5. Classic mechanical plate character

Great for:
- Rich, smooth reverb
- Vintage ambience
- Drums and percussion
- Vocals and instruments
- Classic studio effects


### RingMod

For metallic/bell-like sounds and special effects.

This ring modulator implementation features:

1. Multiple carrier waveforms:
   - Sine: Classic ring mod sound
   - Square: Harsh switching modulation
   - Triangle: Smoother switching
   - Saw: Asymmetric modulation

2. Controls:
   - Frequency: Carrier oscillator frequency
   - Wave: Carrier waveform selection
   - Depth: Modulation amount
   - Offset: DC offset for asymmetric modulation
   - Mix: Wet/dry mix

3. Features:
   - Multiple waveform options
   - Asymmetric modulation capability
   - Output scaling compensation
   - Smooth parameter response
   - Anti-aliased waveforms

Usage tips:
- For classic ring mod: Use sine wave with full depth
- For telephone effects: Use sine wave around 1kHz
- For tremolo effects: Use low frequencies (10-20 Hz)
- For harsh effects: Use square or saw waves
- For subtle modulation: Reduce depth and mix with dry signal
- For asymmetric sounds: Add some offset

Different waveforms create different characters:
- Sine provides the classic, pure ring mod sound
- Square creates harsh, switching effects
- Triangle gives a smoother switching sound
- Saw adds unique asymmetric character

The offset parameter can create interesting effects by adding a DC component to the modulation, which can make the effect more subtle or create asymmetric modulation effects.

The depth parameter controls how much of the carrier signal is applied, and the mix parameter allows for parallel processing effects.


### static

No params, just blasts white-noise.



### Tremolo

Amplitude modulation effect.

This tremolo implementation features:

1. Multiple waveform options:
   - Sine (smooth, classic tremolo)
   - Square (choppy, on/off effect)
   - Triangle (linear transitions)
   - Saw (asymmetric modulation)

2. Controls:
   - Rate: Speed of the modulation
   - Depth: Intensity of the effect
   - Waveform: Shape of the modulation
   - Phase: Stereo offset for wide effects
   - Sync: Option for tempo sync (placeholder)

3. Features:
   - Smooth modulation
   - Stereo phase offset
   - Multiple waveform shapes
   - Anti-aliased waveforms

Usage tips:
- For classic tremolo: Use sine wave with medium rate
- For helicopter effect: Use square wave with fast rate
- For stereo movement: Use phase offset around 180 degrees
- For subtle movement: Use triangle wave with low depth
- For special effects: Try saw wave with various rates

The phase offset parameter allows for stereo width effects when using two channels, creating movement in the stereo field. The multiple waveform options allow for different characters of amplitude modulation, from smooth to choppy.

The sync parameter is included but would need to be integrated with your host's tempo system for full functionality.



### wavefolder

Popular in modular synthesis for harmonic enhancement.

This wavefolder implementation features:

1. Multiple folding algorithms:
   - Sine-based: Smooth, musical folding
   - Triangle: More aggressive, geometric folding
   - Hard clip: Sharp, digital folding
   - Asymmetric: Complex harmonics with bias control

2. Controls:
   - Gain: Input drive amount
   - Folds: Number of folding iterations
   - Type: Folding algorithm selection
   - Bias: Asymmetry control
   - Mix: Wet/dry mix

3. Features:
   - Gain compensation
   - Multiple folding algorithms
   - Asymmetric folding option
   - Soft clipping protection
   - Smooth parameter response

Usage tips:
- For subtle harmonics: Low gain, few folds, sine type
- For aggressive sounds: High gain, many folds, triangle type
- For digital artifacts: Clip type with high folds
- For complex harmonics: Asymmetric type with bias
- For thickness: Mix with dry signal

The different folding algorithms provide various characters:
- Sine folding is smooth and musical
- Triangle folding is more aggressive
- Clip folding is sharp and digital
- Asymmetric folding creates complex harmonics, especially with bias

The bias parameter (when using asymmetric type) can create interesting harmonic content by making the folding asymmetrical. The gain compensation helps maintain reasonable output levels regardless of the amount of folding.

### wavetable

This is a very simple wavetable synth that uses waveforms loaded from the host. It's not the best way to do it (would probably be better with embedded samples) but it shows haow to request data from the host. Similar could be used for a sampler, for example.