'use strict'

var fs = require('fs')
var path = require('path')
var yaml = require('js-yaml');
var silkutil = require('./core/silkutil')
var objectStore = require('./core/object_store')
var path = require('path')

var packageDirMap = {}

module.exports = (client, locale, conditions, eventFilters, configs, commands) => {

  var ObjectProxy = require('./core/object_proxy')(client)
  var InputDialog = require('./views/input_dialog').InputDialog(ObjectProxy)
  var API = require('./core/api')(ObjectProxy)
  var Constants = require('./core/constants')(ObjectProxy)

  // class TabView
  function TabView(id) {
    this.id = id
    this.notifyMethods = new Set(['closeAllTabs', 'closeOtherTabs', 'closeActiveTab', 'addNew', 'setCurrentIndex'])
  }
  
  TabView.prototype = Object.create(ObjectProxy)

  // class TabViewGroup
  function TabViewGroup(id) {
    this.id = id
    this.notifyMethods = new Set(['saveAll', 'splitHorizontally', 'splitVertically'])
  }
  
  TabViewGroup.prototype = Object.create(ObjectProxy)


  // class TextEditView
  function TextEditView(id) {
    this.id = id;
    this.notifyMethods = new Set(['save', 'saveAs', 'undo', 'redo', 'cut', 'copy', 'paste', 'selectAll', 'doDelete', 'moveCursor', 'setThinCursor', 'performCompletion', 'insertNewLineWithIndent', 'indent'])
  }

  TextEditView.prototype = Object.create(ObjectProxy)

  // class Window
  function Window(id) {
    this.id = id
    this.notifyMethods = new Set(['close', 'openFindAndReplacePanel'])
  }
  
  Window.prototype = Object.create(ObjectProxy)

  Window.prototype.statusBar = function() {
    const id = silkutil.callExternalMethod(client, 'statusBar', this.id)
    return id != null ? objectStore.getOrCreate(id, StatusBar) : null
  }


  // class StatusBar
  function StatusBar(id) {
    this.id = id
    this.notifyMethods = new Set(['clearMessage', 'showMessageWithTimeout'])
  }
  
  StatusBar.prototype = Object.create(ObjectProxy)

  StatusBar.prototype.showMessage = function(message, timeout) {
    if (message != null) {
      var timeout = timeout || 0 // ms
      this.showMessageWithTimeout(message, timeout)
    }
  }


  // private utility functions

  function convert(value, name, defaultValue, convertFn) {
    // console.log('value: %s', value)
    if (value != null && convertFn != null) {
      return convertFn(value)
    } else {
      return 'default' in configs[name] ? configs[name].default : defaultValue
    }
  }

// These functions are defined here because this is used by other API.

// translate
const t = (key, defaultValue) => {
  var i, j, packageName, currentObj, fd, translationPaths;
  try {
    // get package name from '<package>:key'
    const semicolonIndex = key.indexOf(':')
    if (semicolonIndex > 0) {
      packageName = key.substring(0, semicolonIndex)
    } else {
      // use 'silkedit' if the package name is missing
      packageName = 'silkedit'
    }

    if (packageName in packageDirMap) {
      translationPaths = []
      translationPaths.push(path.normalize(packageDirMap[packageName] + '/locales/' + locale + "/translation.yml"))
      const indexOf_ = locale.indexOf('_')
      if (indexOf_ > 0) {
        translationPaths.push(path.normalize(packageDirMap[packageName] + '/locales/' + locale.substring(0, indexOf_) + "/translation.yml"))
      }
      for (i = 0; i < translationPaths.length; i++) {
        try {
          fd = fs.openSync(translationPaths[i], 'r')
        } catch(e) {
          // translation file doesn't exist
          if (e.code === 'ENOENT') {
            continue;
          }
          throw e
        }
        if (fd != null) {
          try {
            // get value for the matching key
            const doc = yaml.safeLoad(fs.readFileSync(translationPaths[i], 'utf8'))
            const subKeys = key.substring(semicolonIndex + 1).split('.')
            currentObj = doc
            for (j = 0; j < subKeys.length; j++) {
              if (subKeys[j] in currentObj) {
                currentObj = currentObj[subKeys[j]]
              } else {
                currentObj = null
                break
              }
            }

            if (currentObj != null) {
              return currentObj
            }
          } finally {
            fs.close(fd)
          }
        }
      }
    }
  } catch(e) {
    console.warn(e)
  }

  return defaultValue ? defaultValue : ""
}

const loadPackage = (dir) => {
  var pjson, configPath, doc, module

  fs.readdir(dir, (err, files) => {
    if (err) {
      console.warn(err.message);
      return;
    }

    const packageJsonPath = path.join(dir, "package.json");
    //console.log(packageJsonPath);
    // check if packageJsonPath exists by opening it. fs.exists is deprecated.
    fs.open(packageJsonPath, 'r', (err, fd) => {
      fd && fs.close(fd, (err) => {
        pjson = require(packageJsonPath);
        if (pjson.name == null) {
          console.warn('missing package name')
          return
        }

        // cache a package directory path
        packageDirMap[pjson.name] = dir
         
        // load keymap
        const keymapPath = path.join(dir, "keymap.yml");
        fs.open(keymapPath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            API.loadKeymap(pjson.name, keymapPath);
          })
        })

        // load menu
        const menuFilePath = path.join(dir, "menu.yml");
        fs.open(menuFilePath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            API.loadMenu(pjson.name, menuFilePath);
          })
        })
          
        // load toolbar
        const toolbarFilePath = path.join(dir, "toolbar.yml");
        fs.open(toolbarFilePath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            API.loadToolbar(pjson.name, toolbarFilePath);
          })
        })

        // load config
        configPath = path.join(dir, "config.yml")
        fs.open(configPath, 'r', (err, fd) => {
          if (fd) {
            try {
              if (pjson.name != 'silkedit') {
                API.loadConfig(pjson.name, configPath)
              }
              doc = yaml.safeLoad(fs.readFileSync(configPath, 'utf8'))
              // console.log(doc)
              if ('config' in doc) {
                Object.keys(doc.config).forEach(c => {
                  var configName
                  // don't prepend package name for default package
                  if (pjson.name != 'silkedit') {
                    configName = pjson.name + '.' + c
                  } else {
                    configName = c
                  }
                  configs[configName] = doc.config[c]
                })
              }
            } catch(e) {
              console.warn(e)
            } finally {
              fs.close(fd)
            }
          }

          // register commands
          if (pjson.main) {
            try {
              module = require(dir)
            } catch(e) {
              console.warn(e)
              return
            }
            if (module.commands) {
              if (pjson.name === 'silkedit') {
                // don't add a package prefix for silkedit package
                for (var prop in module.commands) {
                  if (typeof(prop) === 'string') {
                    commands[prop] = module.commands[prop];
                  } else if (Array.isArray(prop) && prop.length == 2 && typeof(prop[0]) == 'string') {
                    commands[prop[0]] = module.commands[prop];
                  }
                }
                API.registerCommands(Object.keys(module.commands).map((cmd) => {
                  return [cmd, t("command." + cmd + ".description")]
                }));
              } else {
                for (var prop in module.commands) {
                  commands[pjson.name + '.' + prop] = module.commands[prop];
                }
                API.registerCommands(Object.keys(module.commands).map(cmd => {
                	  return [pjson.name + '.' + cmd, t(pjson.name + ":command." + cmd + ".description")]
                	}));
              }
            } else {
              console.log("no commands")
            }

            // call module's activate method
            if (module.activate) {
              module.activate()
            }
          }
        })
      })
    })
  })
}

  // API (everything is synchronous)

  // replace these methods with API class methods
  return {
    alert: (msg) => {
      API.alert(msg);
    }
    
    ,loadKeymap: API.loadKeymap

    ,loadMenu: API.loadMenu

    ,loadPackage: loadPackage

    ,registerCommands: API.registerCommands
    
    ,unregisterCommands: API.unregisterCommands

    ,registerCondition: (name, func) => {
      conditions[name] = func
      API.registerCondition(name)
    }

    ,unregisterCondition: (name) => {
      delete conditions[name]
      API.unregisterCondition(name)
    }

    ,activeTextEditView: () => {
      const id = API.activeTextEditView()
      return objectStore.getOrCreate(id, TextEditView)
    }

    ,activeTabView: () => {
      const id = API.activeTabView()
      return objectStore.getOrCreate(id, TabView)
    }

    ,activeTabViewGroup: () => {
      const id = API.activeTabViewGroup()
      return objectStore.getOrCreate(id, TabViewGroup)
    }

    ,activeWindow: () => {
      const id = API.activeWindow()
      return objectStore.getOrCreate(id, Window)
    }

    ,showFileAndFolderDialog: (caption) => {
      caption = caption == null ? 'Open' : caption
      return API.showFileAndFolderDialog(caption)
    }

    ,showFilesDialog: (caption) => {
      caption = caption == null ? 'Open Files' : caption
      return API.showFilesDialog(caption)
    }

    ,showFolderDialog: (caption) => {
      caption = caption == null ? 'Open Folder' : caption
      return API.showFolderDialog(caption)
    }

    ,open: (path) => {
      if (path != null) {
        API.open(path)
      }
    }

    ,dispatchCommand: (keyEvent) => {
      if (keyEvent != null) {
        API.dispatchCommand(keyEvent.type, keyEvent.key, keyEvent.repeat, keyEvent.altKey, keyEvent.ctrlKey, keyEvent.metaKey, keyEvent.shiftKey)
      }
    }

    ,conditionUtils: {
      isSatisfied: (key, operator, value) => {
        switch(operator) {
          case '==':
          return key === value
          case '!=':
          return key !== value
          case '>':
          return key > value
          case '>=':
          return key >= value
          case '<':
          return key < value
          case '<=':
          return key <= value
          default:
          return false
        }
      }
    }

    ,installEventFilter: (type, fn) => {
      if (type in eventFilters) {
        eventFilters[type].push(fn)
      } else {
        eventFilters[type] = [fn]
      }
    }

    ,removeEventFilter: (type, fn) => {
      if (type in eventFilters) {
        const index = eventFilters[type].indexOf(fn)
        if (index !== -1) {
          eventFilters[type].splice(index, 1)
        }
      }
    }
    
    ,windows: () => {
      const ids = API.windows()
      return ids != null ? ids.map(id => objectStore.getOrCreate(id, Window)) : []
    }

    ,config:  {
      get: (name) => {
        if (name in configs) {
          const value = API.getConfig(name)
          const type = configs[name].type
          switch(type) {
            case 'bool':
            case 'boolean':
              return convert(value, name, false, (v) => v === 'true')
            case 'string':
              return convert(value, name, null, (v) => v)
            case 'int':
            case 'integer':
              return convert(value, name, 0, (v) => parseInt(v, 10))
            case 'float':
              return convert(value, name, 0, (v) => parseFloat(v))
            default:
              return null
          }
        } else {
          return null
        }
      }
    }

    ,showInputDialog: (label, initialText, validateFunc) => {
      const dialog = new InputDialog
      dialog.labelText = label
      dialog.textValue = initialText
      if (validateFunc != null) {
        dialog.validateFunc = validateFunc
      }
      return dialog.show()
    }

    ,setFont: (family, size) => {
      family = family == null ? '' : family
      size = size == null ? 0 : size
      API.setFont(family, size)
    }
    ,t: t
    ,Constants: Constants
  }
}