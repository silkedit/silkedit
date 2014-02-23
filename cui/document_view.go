package cui

import (
	"../core"
	"github.com/golang/glog"
	"github.com/nsf/termbox-go"
	"github.com/shinichy/go-wcwidth"
)

type DocumentView struct {
	doc    core.Document
	line   int
	column int
}

func NewDocumentView() *DocumentView {
	view := DocumentView{
		doc:    core.NewDocument(),
		line:   0,
		column: 0,
	}
	view.doc.Subscribe(view.draw)
	return &view
}

func (v *DocumentView) Insert(r rune) {
	glog.Infof("Insert: %v", r)
	if v.doc.Insert(v.column, r) {
		v.column += 1
	}
}

func (v *DocumentView) Delete() {
	if v.doc.Delete(v.column) {
		v.column -= 1
	}
}

func (v *DocumentView) draw() {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	column, line := 0, 0
	v.doc.ForEach(func(r rune) {
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
