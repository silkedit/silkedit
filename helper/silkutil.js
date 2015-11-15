'use strict'

var sync = require('synchronize')

module.exports = {
    runInFiber: (fn) => {
      sync.fiber(() => {
        try {
          fn()
        } catch (err) {
          console.error(err)
        }
      })
    }
}