rpc = require('silk-msgpack-rpc');
fs = require('fs')
sync = require('synchronize')
path = require('path')
yaml = require('js-yaml');

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

const socketFile = process.argv[2];
const moduleFiles = ["menus.yml", "menus.yaml", "package.json"]
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
  GLOBAL.silk = require('./silkedit')(c, contexts, eventFilters, configs);

  sync(c, 'invoke');

  const loadPackage = (dir) => {
    fs.readdir(dir, (err, files) => {
      if (err) {
        console.log(err.message);
        return;
      }

      moduleFiles.forEach((filename) => {
        const filePath = path.join(dir, filename);
        console.log(filePath);
        // check if filePath exists by opening it. fs.exists is deprecated.
        fs.open(filePath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            switch (filename) {
              case "menus.yml":
              case "menus.yaml":
                silk.loadMenu(filePath);
                break;
              case "package.json":
                const pjson = require(filePath);
                if (pjson.name == null) {
                  console.warn('missing package name')
                  return
                }

                const configPath = path.join(dir, "config.yml")
                fs.open(configPath, 'r', (err, fd) => {
                  if (fd) {
                    try {
                      const doc = yaml.safeLoad(fs.readFileSync(configPath, 'utf8'))
                      // console.log(doc)
                      if ('config' in doc) {
                        Object.keys(doc.config).forEach(c => {
                          configs[pjson.name + '.' + c] = doc.config[c]
                        })
                      }
                    } catch(e) {
                      console.warn(e)
                    } finally {
                      fs.close(fd)
                    }
                  }

                  if (pjson.main) {
                    const module = require(dir)
                    if (module.commands) {
                      if (pjson.name === 'silkedit') {
                        // don't add a package prefix for silkedit package
                        for (var prop in module.commands) {
                          commands[prop] = module.commands[prop];
                        }
                        silk.registerCommands(Object.keys(module.commands));
                      } else {
                        for (var prop in module.commands) {
                          commands[pjson.name + '.' + prop] = module.commands[prop];
                        }
                        silk.registerCommands(Object.keys(module.commands).map(c => pjson.name + '.' + c));
                      }
                    } else {
                      console.log("no commands")
                    }

                    if (module.activate) {
                      sync.fiber(() => {
                        module.activate()
                      })
                    }
                  }
                })

                break;
            }
          })
        })
      })
    })
  }

  process.argv.slice(3).forEach((dirPath) => {
    fs.open(dirPath, 'r', (err, fd) => {
      fd && fs.close(fd, (err) => {
        const dirs = getDirs(dirPath);
        dirs.forEach((dir) => {
          loadPackage(path.join(dirPath, dir));
        });    
      })
    })

  })
});

const handler = {
  // notify
  "runCommand": (cmd, args) => {
    if (cmd in commands) {
      sync.fiber(() =>{
        commands[cmd](args)
      })
    }
  }

  // request
  ,"askContext": (name, operator, value, response) => {
      if (name in contexts) {
      sync.fiber(() => {
        response.result(contexts[name](operator, value))
      })
    } else {
      response.result(false)
    }
  }
  ,"eventFilter": (type, event, response) => {
    // console.log('eventFilter. type: %s', type)
    if (type in eventFilters) {
      sync.fiber(() => {
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
      sync.fiber(() => {
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
      sync.fiber(() => {
        const result = eventFilters[type].some(fn => { return fn(event)})
        response.result([result, event.name, event.args])
      })
    } else {
      response.result([false, event.name, event.args])
    }
  }
}
c.setHandler(handler);
