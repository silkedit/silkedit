var rpc = require('msgpack-rpc');
var fs = require('fs')

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

var socketFile = process.argv[2];

var c = rpc.createClient(socketFile, function () {
  GLOBAL.silk = require('./silkedit')(c);

  var home = process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'];
  var packageDirPath = home + "/.silk/Packages";

  var findPackage = function (dir) {
    fs.readdir(dir, function (err, files) {
      if (err) {
        console.log(err.message);
        return;
      }

      files.forEach(function (fileName) {
        var filePath = dir + '/' + fileName
        console.log(filePath)
        fs.stat(filePath, function (err, stat) {
          if (err) {
            console.log(err.message);
            return;
          }

          if (stat) {
            if (stat.isDirectory() && fileName != "node_modules") {
              findPackage(filePath);
            } else if (stat.isFile() && fileName === "package.json") {
              var pjson = require(filePath);
              if (pjson.main) {
                console.log(pjson.main)
                var module = require(dir)
                module.activate()
              }
            }
          }
        })
      })
    })
  };

  findPackage(packageDirPath);
});
