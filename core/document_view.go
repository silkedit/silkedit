package core

import (
	"github.com/golang/glog"
)

type DocumentView struct {
	*Observable
Doc    Document
	line   int
	column int
}

func NewDocumentView() *DocumentView {
	view := DocumentView{
		Observable: NewObservable(),
		Doc:    NewDocument(),
		line:   0,
		column: 0,
	}
	return &view
}

func (v *DocumentView) Insert(r rune) {
	glog.Infof("Insert: %v", r)
	if v.Doc.Insert(v.column, r) {
		v.column += 1
	}
	v.callSubscribers(DOCUMENT_VIEW_INSERT, nil)
}

func (v *DocumentView) Delete() {
	if v.Doc.Delete(v.column) {
		v.column -= 1
	}
	v.callSubscribers(DOCUMENT_VIEW_DELETE, nil)
}

func (v *DocumentView) CursorPos() (column int, line int) {
	return v.column, v.line
}
