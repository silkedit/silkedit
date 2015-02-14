module.exports = function (client) {

  var TextEditView = function (id) {
    this.id = id;
  }

  TextEditView.prototype.text = function () {
    return client.invoke('TextEditView.text', this.id)
  }

  return {
    alert: function (msg) {
      client.notify('alert', msg);
    },

    loadMenu: function (ymlPath) {
      client.notify('load_menu', ymlPath)
    },

    registerCommands: function (commands) {
      client.notify('register_commands', commands)
    },

    activeView: function () {
      return new TextEditView(client.invoke('active_view'))
    }
  }
}