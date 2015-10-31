'use strict'

var fs = require('fs')
var path = require('path')
var yaml = require('js-yaml');
var silkutil = require('./silkutil')
var path = require('path')

var packageDirMap = {}

module.exports = (client, locale, contexts, eventFilters, configs, commands) => {

  var InputDialog = require('./views/input_dialog')(client)

  // class TabView
  const TabView = (id) => {
    this.id = id
  }

  TabView.prototype.closeAllTabs = () => {
    client.notify('TabView.closeAllTabs', this.id)
  }

  TabView.prototype.closeOtherTabs = () => {
    client.notify('TabView.closeOtherTabs', this.id)
  }

  TabView.prototype.closeActiveTab = () => {
    client.notify('TabView.closeActiveTab', this.id)
  }


  TabView.prototype.addNew = () => {
    client.notify('TabView.addNew', this.id)
  }

  // This property holds the number of tabs in the tab bar.
  TabView.prototype.count = () => {
    return client.invoke('TabView.count', this.id)
  }
  
  // This property holds the index of the tab bar's visible tab.
  // The current index is -1 if there is no current tab.
  TabView.prototype.currentIndex = () => {
    return client.invoke('TabView.currentIndex', this.id)
  }
  
  TabView.prototype.setCurrentIndex = (index) => {
    if (typeof(index) == 'number') {
      client.notify('TabView.setCurrentIndex', this.id, index)
    }
  }


  // class TabViewGroup
  const TabViewGroup = (id) => {
    this.id = id
  }

  TabViewGroup.prototype.saveAll = () => {
    client.notify('TabViewGroup.saveAllTabs', this.id)
  }

  TabViewGroup.prototype.splitHorizontally = () => {
    client.notify('TabViewGroup.splitHorizontally', this.id)
  }

  TabViewGroup.prototype.splitVertically = () => {
    client.notify('TabViewGroup.splitVertically', this.id)
  }


  // class TextEditView
  const TextEditView = function (id) {
    this.id = id;
  }

  TextEditView.prototype = {
    text: () => {
      return client.invoke('TextEditView.text', this.id)
    }
    ,save: () => {
      client.notify('TextEditView.save', this.id)
    }
  }

  // todo: Put these functions into a single object like above
  TextEditView.prototype.saveAs = () => {
    client.notify('TextEditView.saveAs', this.id)
  }

  TextEditView.prototype.undo = () => {
    client.notify('TextEditView.undo', this.id)
  }

  TextEditView.prototype.redo = () => {
    client.notify('TextEditView.redo', this.id)
  }

  TextEditView.prototype.cut = () => {
    client.notify('TextEditView.cut', this.id)
  }

  TextEditView.prototype.copy = () => {
    client.notify('TextEditView.copy', this.id)
  }

  TextEditView.prototype.paste = () => {
    client.notify('TextEditView.paste', this.id)
  }

  TextEditView.prototype.selectAll = () => {
    client.notify('TextEditView.selectAll', this.id)
  }

  TextEditView.prototype.delete =  (repeat) => {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    client.notify('TextEditView.delete', this.id, repeat)
  }

  TextEditView.prototype.moveCursor =  (operation, repeat) => {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    if (operation != null && typeof(operation) == 'string') {
      client.notify('TextEditView.moveCursor', this.id, operation, repeat)
    }
  }

  TextEditView.prototype.setThinCursor = (isThin) => {
    client.notify('TextEditView.setThinCursor', this.id, isThin)
  }

  TextEditView.prototype.scopeName = () => {
    return client.invoke('TextEditView.scopeName', this.id)
  }

  TextEditView.prototype.scopeTree = () => {
    return client.invoke('TextEditView.scopeTree', this.id)
  }

  TextEditView.prototype.complete = () => {
    return client.notify('TextEditView.complete', this.id)
  }
  
  TextEditView.prototype.insertNewLine = () => {
    client.notify('TextEditView.insertNewLine', this.id)
  }
  
  TextEditView.prototype.indent = () => {
    client.notify('TextEditView.indent', this.id)
  }

  // class Window
  const Window = (id) => {
    this.id = id
  }

  Window.prototype.close = () => {
    client.notify('Window.close', this.id)
  }

  Window.prototype.openFindPanel = () => {
    client.notify('Window.openFindPanel', this.id)
  }

  Window.prototype.statusBar = () => {
    const id = client.invoke('Window.statusBar', this.id)
    return id != null ? new StatusBar(id) : null
  }


  // class StatusBar
  const StatusBar = (id) => {
    this.id = id
  }

  StatusBar.prototype.showMessage = (message) => {
    if (message != null) {
      client.notify('StatusBar.showMessage', this.id, message)
    }
  }

  StatusBar.prototype.clearMessage = () => {
    client.notify('StatusBar.clearMessage', this.id)
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

const loadMenu = (pkgName, ymlPath) => {
  client.notify('loadMenu', pkgName, ymlPath)
}

const loadToolbar = (pkgName, ymlPath) => {
  client.notify('loadToolbar', pkgName, ymlPath)
}

const registerCommands = (commands) => {
  client.notify('registerCommands', commands)
}

const unregisterCommands = (commands) => {
  client.notify('unregisterCommands', commands)
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

          // load menus
          const menuFilePath = path.join(dir, "menus.yml");
          fs.open(menuFilePath, 'r', (err, fd) => {
            fd && fs.close(fd, (err) => {
              loadMenu(pjson.name, menuFilePath);
            })
          })
          
          // load toolbars
          const toolbarFilePath = path.join(dir, "toolbars.yml");
          fs.open(toolbarFilePath, 'r', (err, fd) => {
            fd && fs.close(fd, (err) => {
              loadToolbar(pjson.name, toolbarFilePath);
            })
          })

          // load configs
          configPath = path.join(dir, "config.yml")
          fs.open(configPath, 'r', (err, fd) => {
            if (fd) {
              try {
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
                    commands[prop] = module.commands[prop];
                  }
                  registerCommands(Object.keys(module.commands));
                } else {
                  for (var prop in module.commands) {
                    commands[pjson.name + '.' + prop] = module.commands[prop];
                  }
                  registerCommands(Object.keys(module.commands).map(c => pjson.name + '.' + c));
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
      client.notify('alert', msg);
    }

    ,loadMenu: loadMenu

    ,loadPackage: loadPackage

    ,registerCommands: registerCommands
    
    ,unregisterCommands: unregisterCommands

    ,registerContext: (name, func) => {
      contexts[name] = func
      client.notify('registerContext', name)
    }

    ,unregisterContext: (name) => {
      delete contexts[name]
      client.notify('unregisterContext', name)
    }

    ,activeView: () => {
      const id = client.invoke('activeView')
      return id != null ? new TextEditView(id) : null
    }

    ,activeTabView: () => {
      const id = client.invoke('activeTabView')
      return id != null ? new TabView(id) : null
    }

    ,activeTabViewGroup: () => {
      const id = client.invoke('activeTabViewGroup')
      return id != null ? new TabViewGroup(id) : null
    }

    ,activeWindow: () => {
      const id = client.invoke('activeWindow')
      return id != null ? new Window(id) : null
    }

    ,showFileAndFolderDialog: (caption) => {
      caption = caption == null ? 'Open' : caption
      return client.invoke('showFileAndFolderDialog', caption)
    }

    ,showFilesDialog: (caption) => {
      caption = caption == null ? 'Open Files' : caption
      return client.invoke('showFilesDialog', caption)
    }

    ,showFolderDialog: (caption) => {
      caption = caption == null ? 'Open Folder' : caption
      return client.invoke('showFolderDialog', caption)
    }

    ,open: (path) => {
      if (path != null) {
        client.notify('open', path)
      }
    }

    ,dispatchCommand: (keyEvent) => {
      if (keyEvent != null) {
        client.notify('dispatchCommand', keyEvent.type, keyEvent.key, keyEvent.repeat, keyEvent.altKey, keyEvent.ctrlKey, keyEvent.metaKey, keyEvent.shiftKey)
      }
    }

    ,contextUtils: {
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
      const ids = client.invoke('windows')
      return ids != null ? ids.map(id => new Window(id)) : []
    }

    ,config:  {
      get: (name) => {
        if (name in configs) {
          const value = client.invoke('getConfig',name)
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

    ,showFontDialog: () => {
      const fontParams = client.invoke('showFontDialog')
      if (fontParams != null) {
        return {
          "family": fontParams[0],
          "size": fontParams[1]
        }
      } else {
        return null
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
      client.notify('setFont', family, size)
    }
    ,t: (key, defaultValue) => {
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

      return defaultValue
    }
  }
}