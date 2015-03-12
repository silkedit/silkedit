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
		,"save": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.save()
			}
		}
		,"save_as": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.saveAs()
			}
		}
		,"save_all": function() {
			var tabViewGroup = silk.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.saveAll()
			}
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
		,"undo": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.undo()
			}
		}
		,"redo": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.redo()
			}
		}
		,"cut": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.cut()
			}
		}
		,"copy": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.copy()
			}
		}
		,"paste": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.paste()
			}
		}
		,"select_all": function() {
			var editView = silk.activeView()
			if (editView != null) {
				editView.selectAll()
			}
		}
		,"delete": function(args) {
			var editView = silk.activeView()
			if (editView != null) {
				if (args['direction'] == 'backward') {
					editView.delete(-1)
				} else {
					editView.delete(1)
				}
			}
		}
	}
}