// this is the unit-management API
// this will also be implemented in C, later (for native)

import waveGenerator from './wave-generator.js'
import UnitIo from './unit-types/UnitIo.js'
import UnitOscillator from './unit-types/UnitOscillator.js'
import UnitPitch from './unit-types/UnitPitch.js'
import UnitScope from './unit-types/UnitScope.js'
import UnitWasm from './unit-types/UnitWasm.js'

// this is the location of worker JS file
const dirWorker = import.meta.url.split('/').slice(0, -1).join('/')

// this is location of wasm units
const dirUnits = `${dirWorker}/units`

// this is the interface to the null-unit system that manages all the units
export default class NullManager {
  constructor (audioCtx) {
    this.audioCtx = audioCtx || new AudioContext()

    // unit 0 is audio-out
    this.units = [
      new UnitIo(this, dirWorker)
    ]

    // each connection
    this.connections = []

    // generate some shared mini-samples (float[256] basic waves)
    this.samples = [
      waveGenerator(0), // sin
      waveGenerator(1), // sqr
      waveGenerator(2), // tri
      waveGenerator(3)  // saw
    ]
  }

  // load a wasm unit
  async load(name) {
    if (!this.units[0].loaded) {
      await this.units[0].load(this)
    }
    const nextID = this.units.length
    switch (name) {
      case 'osc': this.units.push(new UnitOscillator(this)); break
      case 'scope': this.units.push(new UnitScope(this)); break
      case 'pitch': this.units.push(new UnitPitch(this)); break
      default: this.units.push(new UnitWasm(this, name, dirUnits)); break
    }
    this.units[nextID].id = nextID
    await this.units[nextID].load()
    return nextID
  }

  // disconnect all connections for a unit, and unload it
  unload(unitId) {
    // TODO
  }

  // connect a port of a unit to another
  connect(unitId, otherUnitId, portId=0, otherPortId=0) {
    if (this.units[unitId]?.audioNode && this.units[otherUnitId]?.audioNode) {
      this.units[unitId].audioNode.connect(this.units[otherUnitId].audioNode, portId, otherPortId)
    }
  }

  // disconnect a connection of a unit
  disconnect(unitId, otherUnitId, portId = 0, otherPortId = 0) {
    // TODO
  }

  // set a param of a unit
  set_param(unitId, paramId, value) {
    if (this.units[unitId] && this.units[unitId].set_param) {
      this.units[unitId].set_param(paramId, value)
    }
  }

  // get a param of a unit
  async get_param(unitId, paramId) {
    if (this.units[unitId] && this.units[unitId].get_param) {
      return this.units[unitId].get_param(paramId)
    }
  }

  // called on each frame for things that draw
  update() {
    for (const unit of this.units) {
      if (unit.update) {
        unit.update()
      }
    }
    requestAnimationFrame(this.update.bind(this))
  }

  // set the title of a UI, do this before you call ui()
  setTitle(unitId, title) {
    this.units[unitId].title = title
  }

  // web-only: generate a UI for interacting with a unit (or all units)
  ui(unitId) {
    const out = document.createElement('div')
    if (typeof unitId === 'undefined') {
      for (const unit of this.units) {
        if (unit.ui) {
          out.appendChild(unit.ui())
        }
      }
    } else if (this.units[unitId]?.ui) {
      out.appendChild(this.units[unitId].ui())
    }
    return out
  }
}
