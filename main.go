package main

import (
	"./core"
	"flag"
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
	n := v.doc.Insert(uint(v.column), r)
	v.column += n
}

func (v *DocumentView) draw() {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	i := 0
	v.doc.ForEach(func(r rune) {
		termbox.SetCell(i, v.line, r, coldef, coldef)
		i += wcwidth.Wcwidth(r)
	})

	termbox.Flush()
}

func main() {
	flag.Parse()
	glog.Info("Initialize termbox")
	err := termbox.Init()
	if err != nil {
		panic(err)
	}
	defer termbox.Close()
	termbox.SetInputMode(termbox.InputEsc | termbox.InputMouse)

	view := NewDocumentView()
mainloop:
	for {
		switch ev := termbox.PollEvent(); ev.Type {
		case termbox.EventKey:
			switch ev.Key {
			case termbox.KeyEsc:
				break mainloop
			default:
				if ev.Ch != 0 {
					view.Insert(ev.Ch)
				}
			}
		case termbox.EventError:
			panic(ev.Err)
		}
	}

	glog.Flush()
}
