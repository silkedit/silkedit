var fs = require('fs');
var net = require('net');

if (process.argv.length < 3) {
    console.log('missing argument.');
    return;
}

var socketFile = process.argv[2];

var server = net.createServer(function (c) {
  c.write('hello\r\n');
  c.pipe(c);
});

server.on('error', function (e) {
    if (e.code == 'EADDRINUSE') {
        var clientSocket = new net.Socket();
        clientSocket.on('error', function(e) { // handle error trying to talk to server
            if (e.code == 'ECONNREFUSED') {  // No other server listening
                fs.unlinkSync(socketFile);
                server.listen(socketFile, function() { //'listening' listener
                    console.log('server recovered');
                });
            }
        });
        clientSocket.connect({path: socketFile}, function() { 
            console.log('Server running, giving up...');
            process.exit();
        });
    }
});

server.listen(socketFile);