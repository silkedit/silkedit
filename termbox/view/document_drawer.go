package view

type DocumentDrawer interface {
	DrawCursor(int, int, *DocumentView)
	DrawDoc(int, int, *DocumentView)
}
