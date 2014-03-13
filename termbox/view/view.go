package view

type View struct {
	x         int
	y         int
	width     int
	height    int
	maxWidth  int
	maxHeight int
}

func NewView() *View {
	return &View{
		x: 0,
		y: 0,
		width: 0,
		height: 0,
		maxWidth: -1,
		maxHeight: -1,
	}
}

func (v *View) Width() int {
	return v.width
}

func (v *View) Height() int {
	return v.height
}

func (v *View) MaxWidth() int {
	return v.maxWidth
}

func (v *View) MaxHeight() int {
	return v.maxHeight
}

func (v *View) SetHeight(height int) {
	v.height = height
}

func (v *View) SetWidth(width int) {
	v.width = width
}

