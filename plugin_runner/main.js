var rpc = require('msgpack-rpc');
var assert = require('assert');

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

var socketFile = process.argv[2];

var c = rpc.createClient(socketFile, function () {
  var start = Date.now();
  c.invoke('add', 5, 4, function (err, response) {
    var end = Date.now();
    console.log('time', end - start)
    assert.equal(9, response);
    c.close();
  });
});
