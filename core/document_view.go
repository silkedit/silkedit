package core

import (
	"github.com/golang/glog"
)

type DocumentView struct {
	Doc    Document
	drawer Drawer
	line   int
	column int
}

func NewDocumentView(drawer Drawer) *DocumentView {
	view := DocumentView{
		Doc:    NewDocument(),
		drawer: drawer,
		line:   0,
		column: 0,
	}
	drawer.DrawCursor(&view)
	view.Doc.Subscribe(func() {
		drawer.DrawDoc(&view)
	})
	return &view
}

func (v *DocumentView) Insert(r rune) {
	glog.Infof("Insert: %v", r)
	if v.Doc.Insert(v.column, r) {
		v.column += 1
	}
	v.drawer.DrawCursor(v)
}

func (v *DocumentView) Delete() {
	if v.Doc.Delete(v.column) {
		v.column -= 1
	}
	v.drawer.DrawCursor(v)
}

func (v *DocumentView) CursorPos() (column int, line int) {
	return v.column, v.line
}
