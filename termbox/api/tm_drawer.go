package api

import (
	"bitbucket.org/shinichy/sk/core"
	"bitbucket.org/shinichy/sk/termbox/config"
	"github.com/nsf/termbox-go"
	"github.com/shinichy/go-wcwidth"
)

type TermboxDrawer struct{}

func (d TermboxDrawer) DrawCursor(v *core.DocumentView) {
	column, line := 0, 0
	v.Doc.ForEach(func(r rune) {
		if r == '\n' {
			line++
			column = 0
		} else {
			column += wcwidth.Wcwidth(r)
		}
	})
	termbox.SetCursor(column, line)
	termbox.Flush()
}

func (d TermboxDrawer) DrawDoc(v *core.DocumentView) {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	column, line := 0, 0
	v.Doc.ForEach(func(r rune) {
		if r == '\n' {
			line++
			column = 0
		} else {
			termbox.SetCell(column, line, r, coldef, coldef)
			column += wcwidth.Wcwidth(r)
		}
	})
	termbox.Flush()
}
