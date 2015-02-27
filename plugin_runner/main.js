var rpc = require('silk-msgpack-rpc');
var fs = require('fs')
var sync = require('synchronize')
var path = require('path')

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

var socketFile = process.argv[2];
var moduleFiles = ["menus.yml", "menus.yaml", "package.json"]
var commands = {}

function getDirs(dir) {
  var files = fs.readdirSync(dir);
  var dirs = []

  files.forEach(function (file) {
    var stat = fs.statSync(path.join(dir, file));
    if (stat.isDirectory()) {
      dirs.push(file);
    }
  });

  return dirs;
}

var c = rpc.createClient(socketFile, function () {
  GLOBAL.silk = require('./silkedit')(c);

  sync(c, 'invoke');

  var home = process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'];
  var packageDirPath = path.join(home, ".silk", "Packages");

  var loadPackage = function (dir) {
    fs.readdir(dir, function (err, files) {
      if (err) {
        console.log(err.message);
        return;
      }

      moduleFiles.forEach(function (filename) {
        var filePath = path.join(dir, filename);
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
                  if (module.commands) {
                    for (var prop in module.commands) {
                      commands[prop] = module.commands[prop];
                    }
                    silk.registerCommands(Object.keys(module.commands));
                  }

                  if (module.activate) {
                    module.activate()
                  }
                }
                break;
            }
          })
        })
      })
    })
  }

  fs.open(packageDirPath, 'r', function(err, fd) {
    fd && fs.close(fd, function(err) {
      var dirs = getDirs(packageDirPath);
      dirs.forEach(function (dir) {
        loadPackage(path.join(packageDirPath, dir));
      });    
    })
  })
});

var handler = {
  "runCommand": function(cmd) {
    if (commands[cmd]) {
      sync.fiber(function(){
        commands[cmd]()
      })
    }
  }
}
c.setHandler(handler);
