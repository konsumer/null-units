// this is the unit-management API
// this will be implemented in C, later

/*
// name is resolved by host, however, so like "wavetable" is "units/wavetable.wasm" or whatever
unsigned int unit_load(char* name);

void unit_unload(unsigned int unitSource);

bool unit_connect(unsigned int unitSource, unsigned int unitSourcePort, unsigned int unitDestination, unsigned int unitDestinationPort);

bool unit_disconnect(unsigned int unitSource, unsigned int unitSourcePort, unsigned int unitDestination, unsigned int unitDestinationPort);

void unit_set_parm(unsigned int unitSource, unsigned int paramID, NullUnitParamValue* value, float timefromNowInSeconds);

NullUnitParamValue* unit_get_parm(unsigned int unitSource, unsigned int paramID);

NullUnitnInfo* unit_get_info(unsigned int unitSource);
*/

import Oscilloscope from './Oscilloscope.js'
import PitchDetector from './PitchDetector.js'

// this is the location of worker JS file
const dirWorker = import.meta.url.split('/').slice(0, -1).join('/')

// this is location of units
const dirUnits = `${dirWorker}/units`

export default class NullManager {
  constructor(audioCtx) {
    this.audioCtx = audioCtx || new AudioContext()
    this.audioCtx.resume()

    // unit 0 is audio-out
    this.units = [
      { name: 'out', channelsIn: 1, channelsOut: 0, audioNode: this.audioCtx.destination }
    ]
  }

  async load(name) {
    const nextID =  this.units.length

    const newUnit = {
      name,
      params: []
    }

    // add it quickly, so others know the next ID
    this.units.push(newUnit)

    await this.audioCtx.audioWorklet.addModule(`${dirWorker}/unit-processor.js`)
    newUnit.audioNode = new AudioWorkletNode(this.audioCtx, 'null-unit')

    const bytes = await fetch(`${dirUnits}/${name}.wasm`).then(r => r.arrayBuffer())

    // wait for info
    await new Promise((resolve, reject) => {
      newUnit.audioNode.port.onmessage = async e => {
        if (e?.data?.type === 'info' && e.data.id === nextID) {
          newUnit.name = e.data.info.name
          newUnit.channelsIn = e.data.info.channelsIn
          newUnit.channelsOut = e.data.info.channelsOut
          newUnit.params = e.data.info.params
          // console.log('unit loaded', newUnit)
          resolve()
        }
      }
      newUnit.audioNode.port.postMessage({ type: 'load', bytes, id: nextID });
    })

    return nextID
  }

  unload (unitId) {
    // TODO: IDs need to stay the same, so delete (unconnecting it) and set position to undefined
  }

  connect(unitSourceId, unitSourcePort, unitDestinationId, unitDestinationPort) {
    this.units[unitSourceId].audioNode.connect(this.units[unitDestinationId].audioNode, unitSourcePort, unitDestinationPort)
  }

  disconnect(unitSourceId, unitSourcePort, unitDestinationId, unitDestinationPort) {
    // TODO
  }

  set_param(unitSourceId, paramId, value, timefromNowInSeconds=0) {
    // pitchdetector has no options
    if (this.units[unitSourceId].name === 'pitchdetect') {
      return
    }

    // params for osc (tester) follow wavetable
    if (this.units[unitSourceId].name === 'osc') {
      if (paramId === 0 || paramId === 'type') {
        // sin/sqr/tri/saw
        if (Math.floor(value) === 0) {
          this.units[unitSourceId].audioNode.type ='sine'
        }
        if (Math.floor(value) === 1) {
          this.units[unitSourceId].audioNode.type ='square'
        }
        if (Math.floor(value) === 2) {
          this.units[unitSourceId].audioNode.type ='triangle'
        }
        if (Math.floor(value) === 3) {
          this.units[unitSourceId].audioNode.type ='sawtooth'
        }
      }
      if (paramId === 1 || paramId === 'note') {
        // midi note ID
        this.units[unitSourceId].audioNode.frequency.setValueAtTime(440 * Math.pow(2, (value - 69) / 12), timefromNowInSeconds)
      }
      return
    }

    // params for scope look like a normal unit
    if (this.units[unitSourceId].name === 'scope') {
      if (paramId === 'width' || paramId === 0) {
        this.units[unitSourceId].scope.canvas.width = value
      }
      if (paramId === 'height' || paramId === 1) {
        this.units[unitSourceId].scope.canvas.height = value
      }
      if (paramId === 'backgroundColor' || paramId === 2) {
        this.units[unitSourceId].scope.backgroundColor = value
      }
      if (paramId === 'lineColor' || paramId === 3) {
        this.units[unitSourceId].scope.lineColor = value
      }
      if (paramId === 'zeroColor' || paramId === 4) {
        this.units[unitSourceId].scope.zeroColor = value
      }
      if (paramId === 'lineWidth' || paramId === 5) {
        this.units[unitSourceId].scope.lineWidth = value
      }
      if (paramId === 'canvas' || paramId === 6) {
        this.units[unitSourceId].scope.canvas = value
      }
      return
    }

    try {
      // allow string paramId for convenience
      const p = typeof paramId === 'string' ? this.units[unitSourceId].params.findIndex(pi => pi.name === paramId) : paramId
      if (p === -1) {
        throw new Error()
      }

      setTimeout(() => {
        this.units[unitSourceId].audioNode.port.postMessage({ id: unitSourceId, type: 'param_set', paramID: p, value });
      }, timefromNowInSeconds * 1000)
    } catch (e) {
      console.error(`${unitSourceId}.${paramId} not found.`)
    }
  }

  async get_param(unitSourceId, paramId) {
    try {
      // allow string paramId for convenience
      const p = typeof paramId === 'string' ? this.units[unitSourceId].params.findIndex(pi => pi.name === paramId) : paramId
      if (p === -1) {
        throw new Error()
      }
    } catch (e) {
      console.error(`${unitSourceId}.${paramId} not found.`)
    }
  }

  // these are web-specific

  // generate a scope
  // doesn't output canvas (use genui)
  scopeNode() {
    const nextID =  this.units.length
    const scope = new Oscilloscope(this.audioCtx)
    scope.canvas = document.createElement('canvas')
    scope.start()
    this.units.push({name: 'scope', channelsIn: 1, channelsOut: 0, scope, audioNode: scope.destination })
    return nextID
  }

  // tester node that creates an oscillator
  oscNode() {
    const nextID =  this.units.length
    const audioNode = this.audioCtx.createOscillator()
    this.units.push({name: 'osc', channelsIn: 0, channelsOut: 1, audioNode })
    audioNode.start()
    return nextID
  }

  // detect pitch/note of incoming data
  pitchNode() {
    const nextID =  this.units.length
    const detector = new PitchDetector(this.audioCtx)
    detector.start()
    this.units.push({name: 'pitchdetect', channelsIn: 1, channelsOut: 0, detector, audioNode: detector.analyser })
    return nextID
  }

  // this willl return a form DOM object that can be used to control a unit
  genui(unitSourceId) {
    const  f = document.createElement("form")
    f.id=`unit_${unitSourceId}`

    for (const unit of this.units) {
      if (unit.name === 'out') {
        continue
      }

      // scopes have a canvas, just output that
      if (unit.name === 'scope') {
        f.appendChild(unit.scope.canvas)

      // pitchdetect has value for note/frequency, but no display
      } else if (unit.name === 'pitchdetect') {
        const p = document.createElement('pre')
        p.className='pitchdetector'
        unit.detector.onchange = (info) => {
          p.innerHTML = JSON.stringify(info, null, 2)
        }
        f.appendChild(p)

      // wavetable is a bit special, as it has view of waveforms
      } else if (unit.name === 'wavetable') {

      // just add form-elements
      } else {

      }
    }
    f.addEventListener('change', e => {
      // process form-change here
    })

    return f
  }
}
