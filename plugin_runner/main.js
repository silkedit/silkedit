'use strict'

var rpc = require('silk-msgpack-rpc');
var fs = require('fs')
var sync = require('synchronize')
var path = require('path')
var util = require('./util')

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

const socketFile = process.argv[2];
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
  GLOBAL.silk = require('./silkedit')(c, contexts, eventFilters, configs, commands);

  sync(c, 'invoke');

  process.argv.slice(3).forEach((dirPath) => {
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
  "runCommand": (cmd, args) => {
    if (cmd in commands) {
      util.runInFiber(() =>{
        commands[cmd](args)
      })
    }
  }

  ,"focusChanged": (viewType) => {
    var event = {
      "type": viewType
    }
    const type = "focusChanged"
    if (type in eventFilters) {
      util.runInFiber(() =>{
        eventFilters[type].forEach(fn => fn(event))
      })
    }
  }


  // request handlers
  ,"askContext": (name, operator, value, response) => {
      if (name in contexts) {
      util.runInFiber(() => {
        response.result(contexts[name](operator, value))
      })
    } else {
      response.result(false)
    }
  }
  ,"eventFilter": (type, event, response) => {
    // console.log('eventFilter. type: %s', type)
    if (type in eventFilters) {
      util.runInFiber(() => {
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
      util.runInFiber(() => {
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
      util.runInFiber(() => {
        const result = eventFilters[type].some(fn => { return fn(event)})
        response.result([result, event.name, event.args])
      })
    } else {
      response.result([false, event.name, event.args])
    }
  }
}
c.setHandler(handler);
