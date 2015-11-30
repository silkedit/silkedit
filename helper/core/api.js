'use strict';
  
module.exports = (ObjectProxy) => {
  // object API
  function newAPI() {
    this.id = new Buffer('7b32626237643730372d343265332d346265322d613766632d3363363566393937646534307d', 'hex');
    this.notifyMethods = new Set(['loadKeymap', 'loadMenu', 'loadToolbar', 'loadConfig', 'registerCommands', 'unregisterCommands', 'alert', 'registerCondition', 'unregisterCondition', 'open', 'dispatchCommand', 'setFont']);
  }

  newAPI.prototype = Object.create(ObjectProxy)
  return new newAPI
}