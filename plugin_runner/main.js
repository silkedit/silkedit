'use strict'

var rpc = require('silk-msgpack-rpc');
var fs = require('fs')
var sync = require('synchronize')
var path = require('path')
var silkutil = require('./silkutil')
var yaml = require('js-yaml')
var InputDialog = require('./views/input_dialog')(null)
var https = require('https')

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

const socketFile = process.argv[2];
const locale = process.argv[3];
const packagesBeginIndex = 4
var commands = {}
var contexts = {}
var eventFilters = {}
var configs = {}

function getDirs(dir) {
  const files = fs.readdirSync(dir)
  var dirs = []

  files.forEach((file) => {
    const stat = fs.statSync(path.join(dir, file));
    if (stat.isDirectory()) {
      dirs.push(file);
    }
  });

  return dirs;
}

const c = rpc.createClient(socketFile, () => {
  const packagePaths = process.argv.slice(packagesBeginIndex)
  GLOBAL.silk = require('./silkedit')(c, locale, contexts, eventFilters, configs, commands);

  sync(c, 'invoke');
  
  packagePaths.forEach((dirPath) => {
    fs.open(dirPath, 'r', (err, fd) => {
      fd && fs.close(fd, (err) => {
        const dirs = getDirs(dirPath);
        dirs.forEach((dir) => {
          silk.loadPackage(path.join(dirPath, dir));
        });    
      })
    })

  })
});

// Call user's package code in runInFiber!!
const handler = {

  // notify handlers

  "commandEvent": (cmd, args) => {
    var event = {
      "cmd": cmd,
      "args": args
    }
    const type = "command"
    if (type in eventFilters) {
      silkutil.runInFiber(() =>{
        eventFilters[type].forEach(fn => fn(event))
      })
    }
  }

  ,"focusChanged": (viewType) => {
    var event = {
      "type": viewType
    }
    const type = "focusChanged"
    if (type in eventFilters) {
      silkutil.runInFiber(() => {
        eventFilters[type].forEach(fn => fn(event))
      })
    }
  }
  
  ,"InputDialog.textValueChanged": (id, text) => {
    const dialog = InputDialog.getInstance(id)
    if (dialog != null) {
      silkutil.runInFiber(() => {
        dialog.textValueChanged(text)
      })
    }
  }
  ,"loadPackage": (path) => {
    silk.loadPackage(path)
  }


  // request handlers
  
  ,"runCommand": (cmd, args, response) => {
    if (cmd in commands) {
      silkutil.runInFiber(() =>{
        commands[cmd](args)
        response.result(true)
      })
    } else {
      response.result(false)
    }
  }

  ,"askContext": (name, operator, value, response) => {
    if (name in contexts) {
      silkutil.runInFiber(() => {
        response.result(contexts[name](operator, value))
      })
    } else {
      response.result(false)
    }
  }
  ,"eventFilter": (type, event, response) => {
    // console.log('eventFilter. type: %s', type)
    if (type in eventFilters) {
      silkutil.runInFiber(() => {
        response.result(eventFilters[type].some(fn => { return fn(event)}))
      })
    } else {
      response.result(false)
    }
  }
  ,"keyEventFilter": (type, key, repeat, altKey, ctrlKey, metaKey, shiftKey, response) => {
    console.log('keyEventFilter. type: %s, key: %s, repeat: %s, altKey: %s, ctrlKey: %s, metaKey: %s, shiftKey: %s', type, key, repeat, altKey, ctrlKey, metaKey, shiftKey)
    var event = {
      "type": type
      ,"key": key
      ,"repeat": repeat
      ,"altKey": altKey
      ,"ctrlKey": ctrlKey
      ,"metaKey": metaKey
      ,"shiftKey": shiftKey
    }
    if (type in eventFilters) {
      silkutil.runInFiber(() => {
        response.result(eventFilters[type].some((fn) => { return fn(event)}))
      })
    } else {
      response.result(false)
    }
  }
  ,"cmdEventFilter": (name, args, response) => {
    var event = {
      "name": name,
      "args": args
    }
    const type = "runCommand"
    if (type in eventFilters) {
      silkutil.runInFiber(() => {
        const result = eventFilters[type].some(fn => { return fn(event)})
        response.result([result, event.name, event.args])
      })
    } else {
      response.result([false, event.name, event.args])
    }
  }
  ,"translate": (key, defaultText, response) => {
    response.result(silk.t(key, defaultText))
  }
  ,"removePackage": (dir, response) => {
    var pjson, module;
  
    const packageJsonPath = path.join(dir, "package.json");
    fs.open(packageJsonPath, 'r', (err, fd) => {
      fd && fs.close(fd, (err) => {
        pjson = require(packageJsonPath);
        // unregister commands
        if (pjson && pjson.main) {
          try {
            module = require(dir)
            if (module.commands) {
              if (pjson.name === 'silkedit') {
                // don't add a package prefix for silkedit package
                for (var prop in module.commands) {
                  delete commands[prop]
                }
                silk.unregisterCommands(Object.keys(module.commands));
              } else {
                for (var prop in module.commands) {
                  delete commands[pjson.name + '.' + prop]
                }
                silk.unregisterCommands(Object.keys(module.commands).map(c => pjson.name + '.' + c));
              }
            } else {
              console.log("no commands")
            }

            // call module's deactivate method
            if (module.deactivate) {
              silkutil.runInFiber(() => {
                module.deactivate()
              })
            }
            response.result(true)
          } catch(e) {
            console.warn(e)
            response.error(e.message)
          }
        }
      })
    })
  }
  ,"sendGetRequest": (url, response) => {
    https.get(url, function(res) {
      var body = '';
      res.setEncoding('utf8');
 
      res.on('data', function(chunk){
        body += chunk;
      });
 
      res.on('end', function(res){
        response.result(body)
      });
    }).on('error', function(e) {
      response.error(e.message)
    });
  }
}
c.setHandler(handler);
