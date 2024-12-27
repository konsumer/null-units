// simple function to generate a UI from a unit's name/params

export default function genUI(unit) {
  const f = document.createElement('form')
  f.id = `unit_${unit.id}`

  const fs = document.createElement('fieldset')
  f.appendChild(fs)
  const l = document.createElement('legend')
  l.innerText = `${unit.title || unit.name} (${unit.id})`
  fs.appendChild(l)

  for (const param of unit.params) {
    if (param.type === 0) { // bool

    }
    if (param.type === 0) { // boolean
      const i = document.createElement('input')
      param.input = i
      i.type = 'checkbox'
      i.max = param.max
      i.min = param.min
      i.checked = !!param.value
      i.name = param.name
      i.id = param.name

      if ([1, 2].includes(param.type)) { // int
        i.step = 1
      } else {
        i.step = 0.1
      }

      i.addEventListener('change', e => {
        e.target.form.querySelector(`label[for="${param.name}"]`).innerText = `${param.name}: ${e.target.checked ?  'true' : 'false' }`
        unit.set_param(param.name, e.target.checked)
      })

      const l = document.createElement('label')
      l.innerText = `${param.name}: ${param.value ? 'true' : 'false'}`
      l.htmlFor = param.name

      fs.appendChild(l)
      fs.appendChild(i)
    }
    if ([1, 2, 3].includes(param.type)) { // numbers
      const i = document.createElement('input')
      param.input = i
      i.type = 'range'
      i.max = param.max
      i.min = param.min
      i.value = param.value
      i.name = param.name
      i.id = param.name

      if ([1, 2].includes(param.type)) { // int
        i.step = 1
      } else {
        i.step = 0.1
      }

      i.addEventListener('change', e => {
        e.target.form.querySelector(`label[for="${param.name}"]`).innerText = `${param.name}: ${e.target.value}`
        unit.set_param(param.name, e.target.value)
      })

      const l = document.createElement('label')
      l.innerText = `${param.name}: ${param.value}`
      l.htmlFor = param.name

      fs.appendChild(l)
      fs.appendChild(i)
    }
  }
  return f
}
