'use strict'

var objects = {}

module.exports = {
  'getOrCreate': (id, ctor) => {
    if (id in objects) {
      return objects[id]
    } else if (id instanceof Buffer) {
      objects[id] = new ctor(id)
      return objects[id]
    } else {
      console.log('invalid id: ' + id)
      return null
    }
  }
  ,'delete': (id) => { delete objects[id] }
}