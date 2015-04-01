'use strict'

const MODE = {
	CMD: Symbol(),
	INSERT: Symbol(),
	CMDLINE: Symbol()
}

var isEnabled = false
var mode = MODE.CMD
var repeatCount = 0

const keyPressHandler = (event) => {
	console.log('keyPressHandler')
	switch (mode) {
		case MODE.CMD:
			if (event.key) {
				const ch = event.key.charCodeAt(0)
				if ((ch == "0".charCodeAt(0) && repeatCount != 0) || (ch >= "1".charCodeAt(0) && ch <= "9".charCodeAt(0))) {
					repeatCount = repeatCount * 10 + (ch - "0".charCodeAt(0))
					console.log('repeatCount: %d', repeatCount)
					return true
				}

			}
			console.log('dispatchCommand')
			silk.dispatchCommand(event)
			return true
		case MODE.CMDLINE:
			if (event.key == "Escape") {
				setMode(MODE.CMD)
				return true
			}
			break
	}

	return false
}

const runCommandHandler = (event) => {
	console.log('runCommandHandler')
	if (repeatCount > 0) {
		event.args.repeat = repeatCount.toString()
		repeatCount = 0
	}
	return false
}

function toModeText(mode) {
	switch (mode) {
		case MODE.CMD:
			return "normal"
		case MODE.INSERT:
			return "insert"
		case MODE.CMDLINE:
			return "commandline"
		default:
			console.warn('invalid mode: %s', mode)
			return null
	}
}

function enable() {
	silk.on('keypress', keyPressHandler)
	silk.on('runCommand', runCommandHandler)
	silk.registerContext("mode", (operator, value) => {
		console.log('checking mode context')
		return isEnabled && silk.contextUtils.isSatisfied(toModeText(mode), operator, value)
	})
	mode = MODE.CMD
	onModeChanged(mode)
	repeatCount = 0
	isEnabled = true
}

function disable() {
	silk.removeListener('keypress', keyPressHandler)
	silk.unregisterContext("mode")
	const view = silk.activeView();
	if (view != null) {
		view.setThinCursor(true)
	}

	silk.windows().forEach(w => w.statusBar().clearMessage())

	isEnabled = false
}

function onModeChanged(newMode) {
	let text;
	switch (mode) {
		case MODE.CMD:
			text = "CMD"
			break
		case MODE.INSERT:
			text = "INSERT"
			break
		default:
			return
	}

	let win = silk.activeWindow()
	if (win != null) {
		win.statusBar().showMessage(text)
	}

	updateCursor()
}

function updateCursor() {
	const view = silk.activeView();
	if (view != null) {
		const isThin = mode !== MODE.CMD
		view.setThinCursor(isThin)
	}
}

function setMode(newMode) {
	if (mode !== newMode) {
		const view = silk.activeView()
		if (newMode == MODE.INSERT && view != null) {
			view.moveCursor('left')
		}

		mode = newMode
		onModeChanged(newMode)
	}
}

module.exports = {
	activate: () => {
	},

	commands: {
		"toggle_vim_emulation": (args) => {
			if (isEnabled) {
				disable()
			} else {
				enable()
			}
		}
		,"change_mode": (args) => {
			if (!isEnabled) return

			switch(args['mode']) {
				case 'insert':
					setMode(MODE.INSERT)
					break
				case 'normal':
					setMode(MODE.CMD)
					break
				case 'commandline':
					setMode(MODE.CMDLINE)
					break
				default:
					console.warn('invalid mode: ', args['mode'])
					break
			}
		}
	}
}
