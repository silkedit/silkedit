module.exports = {
	activate: function() {
	},

	commands: {
		"word_count": function() {
			var text = silk.activeView().text()
			if (text !== undefined) {
				var count = text.split(" ").filter(function(elem){ return elem !== ""; }).length
				silk.alert(silk.t("word_count:word_count", "word count") + ": " + count)
			} else {
				console.log("text is undefined")
			}
		}
	}
}