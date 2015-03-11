module.exports = {
	activate: function() {
	},

	commands: {
		"new_file": function() {
			var tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.addNew();
			}
		}
		,"open": function() {
			var paths = silk.showFileAndDirectoryDialog('Open')
			paths.forEach(function(path) {
				silk.open(path)
			})
		}
		,"close_all_tabs": function() {
			var tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeAllTabs();
			}
		}
		,"close_other_tabs": function() {
			var tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeOtherTabs();
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