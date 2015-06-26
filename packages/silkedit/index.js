'use strict'

var fs = require('fs-extra')
var path = require('path')

// Replace oldexp (string or regexp) in a file content with newStr
function replaceFileContent(file, oldexp, newStr, cb) {
	fs.readFile(file, 'utf-8', (err, data) => {
		if (err) return cb(err)
					
		const result = data.replace(oldexp, newStr)
		fs.writeFile(file, result, 'utf-8', (err) => {
			if (err) return cb(err)
			cb()
		})
	})
}

module.exports = {
	activate: () => {
		const fontFamily = silk.config.get('font_family')
		const fontSize = silk.config.get('font_size')
		if (fontFamily || fontSize) {
			silk.setFont(fontFamily, fontSize)
		}
	},

	commands: {
		"new_file": () => {
			const tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.addNew();
			}
		}
		,"open": () => {
			const paths = silk.showFileAndDirectoryDialog('Open')
			paths.forEach(function(path) {
				silk.open(path)
			})
		}
		,"open_files": () => {
			const paths = silk.showFilesDialog('Open Files')
			paths.forEach(function(path) {
				silk.open(path)
			})
		}
		,"open_directory": () => {
			const path = silk.showDirectoryDialog('Open Directory')
			if (path != null) {
				silk.open(path)
			}
		}
		,"save": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.save()
			}
		}
		,"save_as": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.saveAs()
			}
		}
		,"save_all": () => {
			const tabViewGroup = silk.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.saveAll()
			}
		}
		,"close_all_tabs": () => {
			const tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeAllTabs();
			}
		}
		,"close_other_tabs": () => {
			const tabView = silk.activeTabView()
			if (tabView != null) {
				tabView.closeOtherTabs();
			}
		}
		,"close_tab": () => {
			const tabView = silk.activeTabView()
			if (tabView != null) {
				if (tabView.count() > 0) {
					tabView.closeActiveTab();
				} else {
					const win = silk.activeWindow()
					if (win != null) {
						win.close()
					}
				}
			}
		}
		,"undo": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.undo()
			}
		}
		,"redo": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.redo()
			}
		}
		,"cut": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.cut()
			}
		}
		,"copy": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.copy()
			}
		}
		,"paste": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.paste()
			}
		}
		,"select_all": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.selectAll()
			}
		}
		,"complete": () => {
			const editView = silk.activeView()
			if (editView != null) {
				editView.complete()
			}
		}
		,"delete": (args) => {
			const editView = silk.activeView()
			if (editView != null) {
				const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1
				if (args['direction'] == 'backward') {
					editView.delete(-1 * repeat)
				} else {
					editView.delete(repeat)
				}
			}
		}
		,"move_cursor": (args) => {
			const editView = silk.activeView()
			const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1
			if (editView != null && 'operation' in args) {
				editView.moveCursor(args['operation'], repeat)
			}
		}
		,"open_find_panel": () => {
			const win = silk.activeWindow()
			if (win != null) {
				win.openFindPanel()
			}
		}
		,"split_horizontally": () => {
			const tabViewGroup = silk.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.splitHorizontally();
			}
		}
		,"split_vertically": () => {
			const tabViewGroup = silk.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.splitVertically();
			}
		}
		,"show_fonts": () => {
			const font = silk.showFontDialog()
			if (font != null) {
				silk.setFont(font.family, font.size)
			}
		}
		,"show_scope": () => {
			const win = silk.activeWindow()
			const editView = silk.activeView()
			if (win != null && editView != null) {
				win.statusBar().showMessage(editView.scopeName())
			}
		}
		,"new_package": () => {
			const pkgPath = silk.showInputDialog("", "Enter new package path", "")
			// copy hello example package to a new package directory
			fs.copy(__dirname + "/resources/hello", pkgPath, (err) => {
				if (err) return console.error(err)

				// replace <name> with the new package name
				// open the package dir as project
				replaceFileContent(pkgPath + "/package.json", /<name>/, path.basename(pkgPath), (err) => {
					if (err) return console.error(err)
					replaceFileContent(pkgPath + "/menus.yml", /<name>/, path.basename(pkgPath), (err) => {
						if (err) return console.error(err)

						silk.open(pkgPath)
						silk.open(pkgPath + '/package.json')
						silk.loadPackage(pkgPath)
					})
				})
			})
		}
	}
}
