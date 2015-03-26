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
var contexts = {}
var eventFilters = {}

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
  GLOBAL.silk = require('./silkedit')(c, contexts, eventFilters);

  sync(c, 'invoke');

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
                  } else {
                    console.log("no commands")
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

  process.argv.slice(3).forEach(function(dirPath) {
    fs.open(dirPath, 'r', function(err, fd) {
      fd && fs.close(fd, function(err) {
        var dirs = getDirs(dirPath);
        dirs.forEach(function (dir) {
          loadPackage(path.join(dirPath, dir));
        });    
      })
    })

  })
});

var handler = {
  // notify
  "runCommand": function(cmd, args) {
    if (cmd in commands) {
      sync.fiber(function(){
        commands[cmd](args)
      })
    }
  }

  // request
  ,"askContext": function(name, operator, value, response) {
      if (name in contexts) {
      sync.fiber(function(){
        response.result(contexts[name](operator, value))
      })
    } else {
      response.result(false)
    }
  }
  ,"eventFilter": function(type, event, response) {
    // console.log('eventFilter. type: %s', type)
    if (type in eventFilters) {
      sync.fiber(function(){
        response.result(eventFilters[type].some(function(fn){ return fn(event)}))
      })
    } else {
      response.result(false)
    }
  }
  ,"cmdEventFilter": function(name, args, response) {
    var event = {
      "name": name,
      "args": args
    }
    var type = "runCommand"
    if (type in eventFilters) {
      sync.fiber(function(){
        var result = eventFilters[type].some(function(fn){
          return fn(event)
        })
        response.result([result, event.name, event.args])
      })
    } else {
      response.result([false, event.name, event.args])
    }
  }
}
c.setHandler(handler);
