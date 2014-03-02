package core

type Drawer interface {
	DrawCursor(v *DocumentView)
	DrawDoc(v *DocumentView)
}
