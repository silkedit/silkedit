package view

import (
	"github.com/nsf/termbox-go"
	"github.com/shinichy/go-wcwidth"
)

type TermboxDrawer struct{}

var tmDrawer = TermboxDrawer{}

func (d TermboxDrawer) DrawCursor(x int, y int, v *DocumentView) {
	column, line := x, y
	iter := v.Doc.Iterator()
	for {
		r, hasNext := iter()
		if !hasNext { break }

		switch r {
		case '\r':
			line++
			column = x
			r, hasNext = iter()
			if r != '\n' {
				continue
			}
		case '\n':
			line++
			column = x
		default:
			column += wcwidth.Wcwidth(r)
		}
	}
	termbox.SetCursor(column, line)
	termbox.Flush()
}

func (d TermboxDrawer) DrawDoc(x int, y int, v *DocumentView) {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	column, line := x, y
	iter := v.Doc.Iterator()
	for {
		r, hasNext := iter()
		if !hasNext { break }

		switch r {
		case '\r':
			line++
			column = x
			r, hasNext = iter()
			if r != '\n' {
				continue
			}
		case '\n':
			line++
			column = x
		default:
			termbox.SetCell(column, line, r, coldef, coldef)
			column += wcwidth.Wcwidth(r)
		}
	}
	termbox.Flush()
}
