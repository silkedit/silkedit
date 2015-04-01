module.exports = function (client, contexts, eventFilters) {

  // class TabView
  var TabView = function(id) {
    this.id = id;
  }

  TabView.prototype.closeAllTabs = function() {
    client.notify('TabView.closeAllTabs', this.id)
  }

  TabView.prototype.closeOtherTabs = function() {
    client.notify('TabView.closeOtherTabs', this.id)
  }

  TabView.prototype.closeActiveTab = function() {
    client.notify('TabView.closeActiveTab', this.id)
  }

  TabView.prototype.addNew = function() {
    client.notify('TabView.addNew', this.id)
  }


  // class TabViewGroup
  var TabViewGroup = function(id) {
    this.id = id
  }

  TabViewGroup.prototype.saveAll = function() {
    client.notify('TabViewGroup.saveAllTabs', this.id)
  }

  TabViewGroup.prototype.splitHorizontally = function() {
    client.notify('TabViewGroup.splitHorizontally', this.id)
  }

  TabViewGroup.prototype.splitVertically = function() {
    client.notify('TabViewGroup.splitVertically', this.id)
  }


  // class TextEditView
  var TextEditView = function (id) {
    this.id = id;
  }

  TextEditView.prototype.text = function () {
    return client.invoke('TextEditView.text', this.id)
  }

  TextEditView.prototype.save = function () {
    client.notify('TextEditView.save', this.id)
  }

  TextEditView.prototype.saveAs = function () {
    client.notify('TextEditView.saveAs', this.id)
  }

  TextEditView.prototype.undo = function () {
    client.notify('TextEditView.undo', this.id)
  }

  TextEditView.prototype.redo = function () {
    client.notify('TextEditView.redo', this.id)
  }

  TextEditView.prototype.cut = function () {
    client.notify('TextEditView.cut', this.id)
  }

  TextEditView.prototype.copy = function () {
    client.notify('TextEditView.copy', this.id)
  }

  TextEditView.prototype.paste = function () {
    client.notify('TextEditView.paste', this.id)
  }

  TextEditView.prototype.selectAll = function () {
    client.notify('TextEditView.selectAll', this.id)
  }

  TextEditView.prototype.delete = function (repeat) {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    client.notify('TextEditView.delete', this.id, repeat)
  }

  TextEditView.prototype.moveCursor = function (operation, repeat) {
    repeat = repeat == null ? 1 : typeof(repeat) == 'number' ? repeat : 1
    if (operation != null && typeof(operation) == 'string') {
      client.notify('TextEditView.moveCursor', this.id, operation, repeat)
    }
  }

  TextEditView.prototype.setThinCursor = function (isThin) {
    client.notify('TextEditView.setThinCursor', this.id, isThin)
  }


  // class Window
  var Window = function(id) {
    this.id = id
  }

  Window.prototype.close = function() {
    client.notify('Window.close', this.id)
  }

  Window.prototype.openFindPanel = function() {
    client.notify('Window.openFindPanel', this.id)
  }

  Window.prototype.statusBar = function() {
    var id = client.invoke('Window.statusBar', this.id)
    return id != null ? new StatusBar(id) : null
  }


  // class StatusBar
  var StatusBar = function(id) {
    this.id = id
  }

  StatusBar.prototype.showMessage = function(message) {
    if (message != null) {
      client.notify('StatusBar.showMessage', this.id, message)
    }
  }


  // API
  return {
    alert: function (msg) {
      client.notify('alert', msg);
    }

    ,loadMenu: function (ymlPath) {
      client.notify('loadMenu', ymlPath)
    }

    ,registerCommands: function (commands) {
      client.notify('registerCommands', commands)
    }

    ,registerContext: function(name, func) {
      contexts[name] = func
      client.notify('registerContext', name)
    }

    ,unregisterContext: function(name) {
      delete contexts[name]
      client.notify('unregisterContext', name)
    }

    ,activeView: function () {
      var id = client.invoke('activeView')
      return id != null ? new TextEditView(id) : null
    }

    ,activeTabView: function () {
      var id = client.invoke('activeTabView')
      return id != null ? new TabView(id) : null
    }

    ,activeTabViewGroup: function () {
      var id = client.invoke('activeTabViewGroup')
      return id != null ? new TabViewGroup(id) : null
    }

    ,activeWindow: function () {
      var id = client.invoke('activeWindow')
      return id != null ? new Window(id) : null
    }

    ,showFileAndDirectoryDialog: function(caption) {
      caption = caption == null ? 'Open' : caption
      return client.invoke('showFileAndDirectoryDialog', caption)
    }

    ,open: function (path) {
      if (path != null) {
        client.notify('open', path)
      }
    }

    ,dispatchCommand: function (keyEvent) {
      if (keyEvent != null) {
        client.notify('dispatchCommand', keyEvent.type, keyEvent.key, keyEvent.repeat, keyEvent.altKey, keyEvent.ctrlKey, keyEvent.metaKey, keyEvent.shiftKey)
      }
    }

    ,contextUtils: {
      isSatisfied: function(key, operator, value) {
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

    ,on: function(type, fn) {
      if (type in eventFilters) {
        eventFilters[type].push(fn)
      } else {
        eventFilters[type] = [fn]
      }
    }

    ,removeListener: function(type, fn) {
      if (type in eventFilters) {
        var index = eventFilters[type].indexOf(fn)
        if (index !== -1) {
          eventFilters[type].splice(index, 1)
        }
      }
    }
  }
}