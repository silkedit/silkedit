'use strict'

var deasync = require('deasync')

module.exports = {
    callExternalMethod: (client, method, id, ...args) => {
          var done = false;
          var result = null;
          var error = null;
          client.invoke.apply(client, [method, id, ...args, (err, res) => {
            done = true;
            if (err) {
              error = err
            } else {
              result = res
            }
          }]);
          deasync.loopWhile(() => !done);
          if (error) {
            throw error;
          }
          return result;
    }
}