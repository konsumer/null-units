export default class Oscilloscope {
    constructor(canvas, width = 600, height = 200) {
        this.canvas = canvas;
        this.canvas.width = width;
        this.canvas.height = height;
        this.ctx = canvas.getContext('2d');
        this.bufferLength = 0;
        this.dataArray = null;

        // Visual settings
        this.backgroundColor = 'rgb(255, 255, 255)';
        this.lineColor = 'rgb(255, 0, 0)';
        this.lineWidth = 2;
    }

    init(audioContext, audioNode) {
        // Create analyzer node
        this.analyser = audioContext.createAnalyser();
        this.analyser.fftSize = 2048;
        this.bufferLength = this.analyser.frequencyBinCount;
        this.dataArray = new Float32Array(this.bufferLength);

        // Connect audio node to analyzer
        audioNode.connect(this.analyser);

        // Start drawing
        this.draw();
    }

    draw() {
        const { ctx, canvas, dataArray, bufferLength, analyser } = this;

        // Request next animation frame
        requestAnimationFrame(() => this.draw());

        // Get waveform data
        analyser.getFloatTimeDomainData(dataArray);

        // Clear canvas
        ctx.fillStyle = this.backgroundColor;
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        // Draw waveform
        ctx.beginPath();
        ctx.lineWidth = this.lineWidth;
        ctx.strokeStyle = this.lineColor;

        const sliceWidth = canvas.width / bufferLength;
        let x = 0;

        for (let i = 0; i < bufferLength; i++) {
            const v = dataArray[i];
            const y = (v + 1) * canvas.height / 2;

            if (i === 0) {
                ctx.moveTo(x, y);
            } else {
                ctx.lineTo(x, y);
            }

            x += sliceWidth;
        }

        ctx.stroke();
    }
}
