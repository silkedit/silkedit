package view

import (
	"github.com/golang/glog"
	"bitbucket.org/shinichy/sk/core"
	"bitbucket.org/shinichy/sk/termbox/events"
)

type DocumentView struct {
	*core.Observable
	*View
	Doc    core.Document
	line   int
	column int
}

var getDrawer = func() DocumentDrawer {
	return tmDrawer
}

func NewDocumentView() *DocumentView {
	view := DocumentView{
		Observable: core.NewObservable(),
		View: NewView(),
		Doc:    core.NewDocument(),
		line:   0,
		column: 0,
	}

	drawer := getDrawer()
	view.Doc.Subscribe(func(ev int, info interface{}) {
		switch ev {
		case core.DOCUMENT_INSERT, core.DOCUMENT_DELETE:
			drawer.DrawDoc(view.x, view.y, &view)
		}
	})
	view.Subscribe(func(ev int, info interface{}) {
		switch ev {
		case events.DOCUMENT_VIEW_INSERT, events.DOCUMENT_VIEW_DELETE:
			drawer.DrawCursor(view.x, view.y, &view)
		}
	})

	return &view
}

func (v *DocumentView) Draw(x int, y int) {
	getDrawer().DrawCursor(x, y, v)
	getDrawer().DrawDoc(x, y, v)
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
	v.CallSubscribers(events.DOCUMENT_VIEW_INSERT, nil)
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

	v.CallSubscribers(events.DOCUMENT_VIEW_DELETE, nil)
}

func (v *DocumentView) CursorPos() (column int, line int) {
	return v.column, v.line
}
