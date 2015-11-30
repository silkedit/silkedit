'use strict'

var Reflect = require('harmony-reflect');
var deasync = require('deasync')

module.exports = (client) => {
  return new Proxy({}, {
    // intercepts proxy[name]
    get: function(target, name, receiver) {      
      if (name === 'notifyMethods' || name === 'id') return Reflect.get(target, name, receiver)
      
      // Fallback to call SilkEdit API
      // Caching notify methods (void return value), we don't need to wait the response from SilkEdit
      if (receiver.notifyMethods && receiver.notifyMethods.has(name)) {
        return (...args) => client.notify.apply(client, [name, receiver.id, ...args])
      } else {
        return (...args) => {
          var done = false
          var result, error
          client.invoke.apply(client, [name, receiver.id, ...args, (err, res) => {
            done = true;
            if (err) {
              error = err
            } else {
              result = res
            }
          }])
          deasync.loopWhile(() => !done)
          if (error) {
            throw error
          }
          return result
        }
      }
    }
  });
}