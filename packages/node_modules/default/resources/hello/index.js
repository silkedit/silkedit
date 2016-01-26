const silkedit = require('silkedit');

module.exports = {
  activate: function() {
  },

  deactivate: function() {
  },

  commands: {
    "hello": () => {
      silkedit.alert(silkedit.tr("<name>:hello", "Hello!"));
    }
  }
}
