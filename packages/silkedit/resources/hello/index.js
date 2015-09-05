module.exports = {
	activate: function() {
	},

	commands: {
		"hello": () => {
			silk.alert(silk.t("<name>:hello"));
		}
	}
}
