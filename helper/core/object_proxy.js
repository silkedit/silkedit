'use strict'

var Reflect = require('harmony-reflect');
var deasync = require('deasync')
var silkutil = require('./silkutil')

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
        return (...args) => silkutil.callExternalMethod(client, name, receiver.id, ...args)
      }
    }
  });
}