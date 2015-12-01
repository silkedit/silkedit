'use strict';
  
module.exports = (ObjectProxy) => {
  // object Constants
  function newConstants() {
  }

  newConstants.prototype = Object.create(ObjectProxy);
  newConstants.prototype.id = new Buffer('7b36336433393166332d653764382d343261342d393339622d3435373934663265333532367d', 'hex');
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