'use strict'

var Reflect = require('harmony-reflect');

module.exports = (client) => {
  return new Proxy({}, {
    // intercepts proxy[name]
    get: function(target, name, receiver) {      
      if (name === 'prototype') return Object.prototype;
      
      if (name in receiver.prototype) {
        return Reflect.get(target, name, receiver)
      }
      
      // Caching notify methods (void return value), we don't need to wait the response from SilkEdit
      if (receiver.notifyMethods.has(name)) {
        return (...args) => client.notify.apply(client, [name, receiver.id, ...args])
      } else {
        return (...args) => {return client.invoke.apply(client, [name, receiver.id, ...args])}
      }
    }
  });
}