module.exports = {
	activate: function() {
	},

	commands: {
		"close_all_tabs": function() {
			var tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeAllTabs();
			}
		}
		,"close_active_tab": function() {
			var tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeActiveTab();
			} else {
				var win = silk.activeWindow()
				if (win != null) {
					win.close()
				}
			}
		}
	}
}