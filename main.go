package main

import (
	"./core"
	"github.com/nsf/termbox-go"
	"unicode/utf8"
)

type DocumentView struct {
	doc    core.Document
	line   int
	column int
}

func (v *DocumentView) Insert(r rune) {
	buf := make([]byte, 4)
	n := utf8.EncodeRune(buf, r)
	for i := 0; i < n; i++ {
		v.doc.Insert(uint(v.column), buf[i])
		v.column++
	}

	v.draw()
}

func (v *DocumentView) draw() {
	const coldef = termbox.ColorDefault
	termbox.Clear(coldef, coldef)
	v.doc.ForEach(func(i int, b byte) {
		termbox.SetCell(i, v.line, rune(b), coldef, coldef)
	})

	termbox.Flush()
}

func main() {
	err := termbox.Init()
	if err != nil {
		panic(err)
	}
	defer termbox.Close()
	termbox.SetInputMode(termbox.InputEsc | termbox.InputMouse)

	view := DocumentView{
		doc:    core.NewDocument(),
		line:   0,
		column: 0,
	}

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
}
