var net = require('net');
var msgpack = require('msgpack5')() // namespace our extensions
  , encode  = msgpack.encode
  , decode  = msgpack.decode

if (process.argv.length < 3) {
    console.log('missing argument.');
    return;
}

var socketFile = process.argv[2];

var client = net.connect(socketFile, function() {
    console.log('connected to server!');
    var data = encode('hello');
    // console.log(data.toString('hex'));
    client.write(data);
});
