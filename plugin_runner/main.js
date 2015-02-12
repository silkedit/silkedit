var rpc = require('msgpack-rpc');
var fs = require('fs')

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

var socketFile = process.argv[2];
var moduleFiles = ["menus.yml", "menus.yaml", "package.json"]

function getDirs(dir) {
  var files = fs.readdirSync(dir);
  var dirs = []

  files.forEach(function(file) {
    var stat = fs.statSync(dir + "/" + file);
    if (stat.isDirectory()) {
      dirs.push(file);
    }
  });

  return dirs;
}

var c = rpc.createClient(socketFile, function () {
  GLOBAL.silk = require('./silkedit')(c);

  var home = process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'];
  var packageDirPath = home + "/.silk/Packages";

  var loadPackage = function (dir) {
    fs.readdir(dir, function (err, files) {
      if (err) {
        console.log(err.message);
        return;
      }

      moduleFiles.forEach(function (filename) {
        var filePath = dir + '/' + filename;
        console.log(filePath);
        // check if filePath exists by opening it. fs.exists is deprecated.
        fs.open(filePath, 'r', function (err, fd) {
          fd && fs.close(fd, function (err) {
            switch (filename) {
              case "menus.yml":
              case "menus.yaml":
                silk.loadMenu(filePath);
                break;
              case "package.json":
                var pjson = require(filePath);
                if (pjson.main) {
                  var module = require(dir)
                  module.activate()
                }
                break;
            }
          });
        });
      });
    })
  };

  var dirs = getDirs(packageDirPath);
  dirs.forEach(function(dir) {
    loadPackage(packageDirPath + '/' + dir);
  });
});
