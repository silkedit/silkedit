module.exports = function (client) {

  // class TabView
  var TabView = function(id) {
    this.id = id;
  }

  TabView.prototype.closeAllTabs = function() {
    client.notify('TabView.close_all_tabs', this.id)
  }

  TabView.prototype.closeOtherTabs = function() {
    client.notify('TabView.close_other_tabs', this.id)
  }

  TabView.prototype.closeActiveTab = function() {
    client.notify('TabView.close_active_tab', this.id)
  }

  TabView.prototype.addNew = function() {
    client.notify('TabView.add_new', this.id)
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
      client.notify('load_menu', ymlPath)
    }

    ,registerCommands: function (commands) {
      client.notify('register_commands', commands)
    }

    ,activeView: function () {
      var id = client.invoke('active_view')
      return id != null ? new TextEditView(id) : null
    }

    ,activeTabView: function () {
      var id = client.invoke('active_tab_view')
      return id != null ? new TabView(id) : null
    }

    ,activeWindow: function () {
      var id = client.invoke('active_window')
      return id != null ? new Window(id) : null
    }
  }
}