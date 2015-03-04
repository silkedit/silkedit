module.exports = {
	activate: function() {
	},

	commands: {
		"word_count.word_count": function() {
			var text = silk.activeView().text()
			if (text !== undefined) {
				var count = text.split(" ").filter(function(elem){ return elem !== ""; }).length
				silk.alert("word count: " + count)
			} else {
				console.log("text is undefined")
			}
		}
	}
}