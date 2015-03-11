module.exports = function (client) {

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

  // class TextEditView
  var TextEditView = function (id) {
    this.id = id;
  }

  TextEditView.prototype.text = function () {
    return client.invoke('TextEditView.text', this.id)
  }

  // class Window
  var Window = function(id) {
    this.id = id
  }

  Window.prototype.close = function() {
    client.notify('Window.close', this.id)
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

    ,activeView: function () {
      var id = client.invoke('activeView')
      return id != null ? new TextEditView(id) : null
    }

    ,activeTabView: function () {
      var id = client.invoke('activeTabView')
      return id != null ? new TabView(id) : null
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
  }
}