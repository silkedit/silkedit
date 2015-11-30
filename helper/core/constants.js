'use strict';
  
module.exports = (ObjectProxy) => {
  // object Constants
  function newConstants() {
  }

  newConstants.prototype = Object.create(ObjectProxy);
  newConstants.prototype.id = -2;
  newConstants.prototype.notifyMethods = new Set([]);
  const Constants = new newConstants;

  ['userPackagesJsonPath', 'userPackagesNodeModulesPath'].forEach((p) => {
    Object.defineProperty(Constants, p, {
      get: function() {
        // get actual value via proxy
        return newConstants.prototype[p]()
      }
    });
  })

  return Constants;
}