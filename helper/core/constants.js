'use strict'
  
module.exports = (ObjectProxy) => {
  // object Constants
  function newConstants() {
  }

  newConstants.prototype = Object.create(ObjectProxy)
  newConstants.prototype.id = -2
  newConstants.prototype.notifyMethods = new Set([])
  const Constants = new newConstants
  // define userPackagesJsonPath as property (can't use proxy)
  Object.defineProperty(Constants, 'userPackagesJsonPath', {
    get: function() {
      // get actual value via proxy
      return newConstants.prototype.userPackagesJsonPath()
    }
  });
  return Constants
}