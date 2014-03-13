package view

type Drawer interface {
	Draw(x int, y int)
	Height() int
	Width() int
	SetHeight(int)
	SetWidth(int)
	MaxHeight() int
	MaxWidth() int
}
