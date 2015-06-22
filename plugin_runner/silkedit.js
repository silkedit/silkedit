'use strict'

var fs = require('fs')
var path = require('path')
var yaml = require('js-yaml');
var util = require('./util')
var path = require('path')

const moduleFiles = ["menus.yml", "menus.yaml", "package.json"]

module.exports = (client, contexts, eventFilters, configs, commands) => {

  // class TabView
  const TabView = (id) => {
    this.id = id;
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

  TabView.prototype.count = () => {
    return client.invoke('TabView.count', this.id)
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

  TextEditView.prototype.text = () => {
    return client.invoke('TextEditView.text', this.id)
  }

  TextEditView.prototype.save = () => {
    client.notify('TextEditView.save', this.id)
  }

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

  TextEditView.prototype.complete = () => {
    return client.notify('TextEditView.complete', this.id)
  }
  
  TextEditView.prototype.insertNewLine = () => {
    client.notify('TextEditView.insertNewLine', this.id)
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

const loadMenu = (ymlPath) => {
  client.notify('loadMenu', ymlPath)
}

const registerCommands = (commands) => {
  client.notify('registerCommands', commands)
}

const loadPackage = (dir) => {
    var pjson, configPath, doc, module

    fs.readdir(dir, (err, files) => {
      if (err) {
        console.warn(err.message);
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
                loadMenu(filePath);
                break;
              case "package.json":
                pjson = require(filePath);
                if (pjson.name == null) {
                  console.warn('missing package name')
                  return
                }

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

                  if (pjson.main) {
                    module = require(dir)
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

                    if (module.activate) {
                      util.runInFiber(() => {
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

  // API

  return {
    alert: (msg) => {
      client.notify('alert', msg);
    }

    ,loadMenu: loadMenu

    ,loadPackage: loadPackage

    ,registerCommands: registerCommands

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

    ,showFileAndDirectoryDialog: (caption) => {
      caption = caption == null ? 'Open' : caption
      return client.invoke('showFileAndDirectoryDialog', caption)
    }

    ,showFilesDialog: (caption) => {
      caption = caption == null ? 'Open Files' : caption
      return client.invoke('showFilesDialog', caption)
    }

    ,showDirectoryDialog: (caption) => {
      caption = caption == null ? 'Open Directory' : caption
      return client.invoke('showDirectoryDialog', caption)
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

    ,on: (type, fn) => {
      if (type in eventFilters) {
        eventFilters[type].push(fn)
      } else {
        eventFilters[type] = [fn]
      }
    }

    ,removeListener: (type, fn) => {
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

    ,showInputDialog: (title, label, initialText) => {
      const pkgDirPath = packageDir()
      return client.invoke('showInputDialog', title, label, path.normalize(pkgDirPath + '/my_package'))
    }

    ,setFont: (family, size) => {
      family = family == null ? '' : family
      size = size == null ? 0 : size
      client.notify('setFont', family, size)
    }
  }
}