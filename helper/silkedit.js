'use strict'

var fs = require('fs')
var path = require('path')
var yaml = require('js-yaml');
var silkutil = require('./silkutil')
var path = require('path')

var packageDirMap = {}

module.exports = (client, locale, conditions, eventFilters, configs, commands) => {

  var InputDialog = require('./views/input_dialog')(client)

  // class TabView
  function TabView(id) {
    this.id = id
  }

  TabView.prototype.closeAllTabs = function() {
    client.notify('closeAllTabs', this.id)
  }

  TabView.prototype.closeOtherTabs = function() {
    client.notify('closeOtherTabs', this.id)
  }

  TabView.prototype.closeActiveTab = function() {
    client.notify('closeActiveTab', this.id)
  }


  TabView.prototype.addNew = function() {
    client.notify('addNew', this.id)
  }

  // This property holds the number of tabs in the tab bar.
  TabView.prototype.count = function() {
    return client.invoke('count', this.id)
  }
  
  // This property holds the index of the tab bar's visible tab.
  // The current index is -1 if there is no current tab.
  TabView.prototype.currentIndex = function() {
    return client.invoke('currentIndex', this.id)
  }
  
  TabView.prototype.setCurrentIndex = function(index) {
    if (typeof(index) == 'number') {
      client.notify('setCurrentIndex', this.id, index)
    }
  }


  // class TabViewGroup
  function TabViewGroup(id) {
    this.id = id
  }

  TabViewGroup.prototype.saveAll = function() {
    client.notify('saveAllTabs', this.id)
  }

  TabViewGroup.prototype.splitHorizontally = function() {
    client.notify('splitHorizontally', this.id)
  }

  TabViewGroup.prototype.splitVertically = function() {
    client.notify('splitVertically', this.id)
  }


  // class TextEditView
  function TextEditView(id) {
    this.id = id;
  }

  TextEditView.prototype = {
    text: function() {
      return client.invoke('toPlainText', this.id)
    }
    ,save: function() {
      client.notify('save', this.id)
    }
  }

  // todo: Put these functions into a single object like above
  TextEditView.prototype.saveAs = function() {
    client.notify('saveAs', this.id)
  }

  TextEditView.prototype.undo = function() {
    client.notify('undo', this.id)
  }

  TextEditView.prototype.redo = function() {
    client.notify('redo', this.id)
  }

  TextEditView.prototype.cut = function() {
    client.notify('cut', this.id)
  }

  TextEditView.prototype.copy = function() {
    client.notify('copy', this.id)
  }

  TextEditView.prototype.paste = function() {
    client.notify('paste', this.id)
  }

  TextEditView.prototype.selectAll = function() {
    client.notify('selectAll', this.id)
  }

  TextEditView.prototype.delete = function(repeat) {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    client.notify('doDelete', this.id, repeat)
  }

  TextEditView.prototype.moveCursor = function(operation, repeat) {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    if (operation != null && typeof(operation) == 'string') {
      client.notify('moveCursor', this.id, operation, repeat)
    }
  }

  TextEditView.prototype.setThinCursor = function(isThin) {
    client.notify('setThinCursor', this.id, isThin)
  }

  TextEditView.prototype.scopeName = function() {
    return client.invoke('scopeName', this.id)
  }

  TextEditView.prototype.scopeTree = function() {
    return client.invoke('scopeTree', this.id)
  }

  TextEditView.prototype.complete = function() {
    return client.notify('performCompletion', this.id)
  }
  
  TextEditView.prototype.insertNewLine = function() {
    client.notify('insertNewLineWithIndent', this.id)
  }
  
  TextEditView.prototype.indent = function() {
    client.notify('indent', this.id)
  }

  // class Window
  function Window(id) {
    this.id = id
  }

  Window.prototype.close = function() {
    client.notify('close', this.id)
  }

  Window.prototype.openFindPanel = function() {
    client.notify('openFindAndReplacePanel', this.id)
  }

  Window.prototype.statusBar = function() {
    const id = client.invoke('statusBar', this.id)
    return id != null ? new StatusBar(id) : null
  }


  // class StatusBar
  function StatusBar(id) {
    this.id = id
  }

  StatusBar.prototype.showMessage = function(message, timeout) {
    if (message != null) {
      var timeout = timeout || 0 // ms
      client.notify('showMessage', this.id, message, timeout)
    }
  }

  StatusBar.prototype.clearMessage = function() {
    client.notify('clearMessage', this.id)
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

// Returns SilkEdit package directory path.
const packageDir = () => {
  const home = process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME']
  return path.normalize(home + '/.silk/packages')
}

const loadKeymap = (pkgName, ymlPath) => {
  client.notify('loadKeymap', -1, pkgName, ymlPath)
}

const loadMenu = (pkgName, ymlPath) => {
  client.notify('loadMenu', -1, pkgName, ymlPath)
}

const loadToolbar = (pkgName, ymlPath) => {
  client.notify('loadToolbar', -1, pkgName, ymlPath)
}

const loadConfig = (pkgName, ymlPath) => {
  client.notify('loadConfig', -1, pkgName, ymlPath)
}

const registerCommands = (commands) => {
  client.notify('registerCommands', -1, commands)
}

const unregisterCommands = (commands) => {
  client.notify('unregisterCommands', -1, commands)
}


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
            loadKeymap(pjson.name, keymapPath);
          })
        })

        // load menu
        const menuFilePath = path.join(dir, "menu.yml");
        fs.open(menuFilePath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            loadMenu(pjson.name, menuFilePath);
          })
        })
          
        // load toolbar
        const toolbarFilePath = path.join(dir, "toolbar.yml");
        fs.open(toolbarFilePath, 'r', (err, fd) => {
          fd && fs.close(fd, (err) => {
            loadToolbar(pjson.name, toolbarFilePath);
          })
        })

        // load config
        configPath = path.join(dir, "config.yml")
        fs.open(configPath, 'r', (err, fd) => {
          if (fd) {
            try {
              if (pjson.name != 'silkedit') {
                loadConfig(pjson.name, configPath)
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
                registerCommands(Object.keys(module.commands).map((cmd) => {
                  return [cmd, t("command." + cmd + ".description")]
                }));
              } else {
                for (var prop in module.commands) {
                  commands[pjson.name + '.' + prop] = module.commands[prop];
                }
                registerCommands(Object.keys(module.commands).map(cmd => {
                	  return [pjson.name + '.' + cmd, t(pjson.name + ":command." + cmd + ".description")]
                	}));
              }
            } else {
              console.log("no commands")
            }

            // call module's activate method
            if (module.activate) {
              silkutil.runInFiber(() => {
                module.activate()
              })
            }
          }
        })
      })
    })
  })
}

  // API (everything is synchronous)

  return {
    alert: (msg) => {
      client.notify('alert', -1, msg);
    }
    
    ,loadKeymap: loadKeymap

    ,loadMenu: loadMenu

    ,loadPackage: loadPackage

    ,registerCommands: registerCommands
    
    ,unregisterCommands: unregisterCommands

    ,registerCondition: (name, func) => {
      conditions[name] = func
      client.notify('registerCondition', -1, name)
    }

    ,unregisterCondition: (name) => {
      delete conditions[name]
      client.notify('unregisterCondition', -1, name)
    }

    ,activeTextEditView: () => {
      const id = client.invoke('activeTextEditView', -1)
      return id != null ? new TextEditView(id) : null
    }

    ,activeTabView: () => {
      const id = client.invoke('activeTabView', -1)
      return id != null ? new TabView(id) : null
    }

    ,activeTabViewGroup: () => {
      const id = client.invoke('activeTabViewGroup', -1)
      return id != null ? new TabViewGroup(id) : null
    }

    ,activeWindow: () => {
      const id = client.invoke('activeWindow', -1)
      return id != null ? new Window(id) : null
    }

    ,showFileAndFolderDialog: (caption) => {
      caption = caption == null ? 'Open' : caption
      return client.invoke('showFileAndFolderDialog', -1, caption)
    }

    ,showFilesDialog: (caption) => {
      caption = caption == null ? 'Open Files' : caption
      return client.invoke('showFilesDialog', -1, caption)
    }

    ,showFolderDialog: (caption) => {
      caption = caption == null ? 'Open Folder' : caption
      return client.invoke('showFolderDialog', -1, caption)
    }

    ,open: (path) => {
      if (path != null) {
        client.notify('open', -1, path)
      }
    }

    ,dispatchCommand: (keyEvent) => {
      if (keyEvent != null) {
        client.notify('dispatchCommand', -1, keyEvent.type, keyEvent.key, keyEvent.repeat, keyEvent.altKey, keyEvent.ctrlKey, keyEvent.metaKey, keyEvent.shiftKey)
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
      const ids = client.invoke('windows', -1)
      console.log('windows ' + ids)
      return ids != null ? ids.map(id => new Window(id)) : []
    }

    ,config:  {
      get: (name) => {
        if (name in configs) {
          const value = client.invoke('getConfig', -1, name)
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

    ,"packageDir": packageDir

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
      client.notify('setFont', -1, family, size)
    }
    ,t: t
  }
}