package view

import (
	termbox "github.com/nsf/termbox-go"
	wcwidth "github.com/shinichy/go-wcwidth"
)

type StatusView struct {
	*View
	Mode string
}

func NewStatusView() *StatusView {
	view := NewView()
	view.maxHeight = 1
	return &StatusView{
		View: view,
		Mode: "Insert",
	}
}

func (v *StatusView) Draw(px int, py int) {
	const coldef = termbox.ColorDefault
	x, y := px, py
	for _, r := range v.Mode {
		termbox.SetCell(x, y, r, coldef, coldef)
		x += wcwidth.Wcwidth(r)
	}
}
