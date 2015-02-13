module.exports = function (client) {

  var TextEditView = function (id) {
    this.id = id;
  }

  TextEditView.prototype.getText = function () {
    return client.invoke('TextEditView.getText', this.id)
  }

  return {
    alert: function (msg) {
      client.notify('alert', msg);
    },

    loadMenu: function (ymlPath) {
      client.notify('loadMenu', ymlPath)
    },

    registerCommands: function (commands) {
      client.notify('registerCommands', commands)
    },

    activeView: function () {
      return new TextEditView(client.invoke('getActiveView'))
    }
  }
}