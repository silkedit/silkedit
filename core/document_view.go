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

func (v *DocumentView) InsertString(s string) {
	for _, r := range s {
		v.Insert(r)
	}
}

func (v *DocumentView) Insert(r rune) {
	glog.Infof("Insert: %v", r)
	if v.Doc.Insert(v.column, r) {
		v.column += 1
	}
	v.callSubscribers(DOCUMENT_VIEW_INSERT, nil)
}

func (v *DocumentView) Delete() {
	if v.column <= 0 {
		return
	}

	delCount := 1
	if v.Doc.Get(v.column-1) == '\n' && v.column-2 >= 0 && v.Doc.Get(v.column-2) == '\r' {
		delCount = 2
	}

	for i := 0; i < delCount; i++ {
		if v.Doc.Delete(v.column) {
			v.column -= 1
		}
	}

	v.callSubscribers(DOCUMENT_VIEW_DELETE, nil)
}

func (v *DocumentView) CursorPos() (column int, line int) {
	return v.column, v.line
}
