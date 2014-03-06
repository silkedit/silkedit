package api

import (
	"bitbucket.org/shinichy/sk/core"
	"github.com/nsf/termbox-go"
	"github.com/shinichy/go-wcwidth"
)

type TermboxDrawer struct{}

func (d TermboxDrawer) DrawCursor(v *core.DocumentView) {
	column, line := 0, 0
	iter := v.Doc.Iterator()
	for {
		r, hasNext := iter()
		if !hasNext { break }

		switch r {
		case '\r':
			line++
			column = 0
			r, hasNext = iter()
			if r != '\n' {
				continue
			}
		case '\n':
			line++
			column = 0
		default:
			column += wcwidth.Wcwidth(r)
		}
	}
	termbox.SetCursor(column, line)
	termbox.Flush()
}

func (d TermboxDrawer) DrawDoc(v *core.DocumentView) {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	column, line := 0, 0
	iter := v.Doc.Iterator()
	for {
		r, hasNext := iter()
		if !hasNext { break }

		switch r {
		case '\r':
			line++
			column = 0
			r, hasNext = iter()
			if r != '\n' {
				continue
			}
		case '\n':
			line++
			column = 0
		default:
			termbox.SetCell(column, line, r, coldef, coldef)
			column += wcwidth.Wcwidth(r)
		}
	}
	termbox.Flush()
}
