'use strict'
  
module.exports = (ObjectProxy) => {
  // object API
function newAPI() {
  this.id = -1
  this.notifyMethods = new Set(['loadKeymap', 'loadMenu', 'loadToolbar', 'loadConfig', 'registerCommands', 'unregisterCommands', 'alert', 'registerCondition', 'unregisterCondition', 'open', 'dispatchCommand', 'setFont'])
}
  newAPI.prototype = Object.create(ObjectProxy)
  return new newAPI
}