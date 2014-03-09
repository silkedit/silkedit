package view

type Drawer interface {
	DrawCursor(*DocumentView)
	DrawDoc(*DocumentView)
}
