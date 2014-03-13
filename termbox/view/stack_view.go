package view

type StackView struct {
	*View
	drawers []Drawer
}

func NewStackView() *StackView {
	return &StackView{
		View: NewView(),
		drawers: make([]Drawer, 0),
	}
}

func (v *StackView) Add(d Drawer) {
	v.drawers = append(v.drawers, d)
}

func (v *StackView) Height() int {
	h := 0
	for _, d := range v.drawers {
		h += d.Height()
	}
	return h
}

func (v *StackView) Width() int {
	w := 0
	for _, d := range v.drawers {
		if w < d.Width() {
			w = d.Width()
		}
	}
	return w
}

func (v *StackView) Draw(px int, py int) {
	v.assignSize()
	x , y := px, py
	for _, d := range v.drawers {
		d.Draw(x, y)
		y += d.Height()
	}
}

func (v *StackView) assignSize() {
	// decide height of each child drawer
	h := v.height
	group1, group2 := span(v.drawers, func(dw Drawer) bool {
			return dw.MaxHeight() >= 0
		})

	for _, d := range group1 {
		if h >= d.MaxHeight() {
			d.SetHeight(d.MaxHeight())
			h -= d.MaxHeight()
		} else {
			d.SetHeight(h)
			h = 0
		}
	}

	if (len(group2) > 0) {
		splitHeight := h / len(group2)
		for _, d := range group2 {
			d.SetHeight(splitHeight)
		}
	}

	// decide width of each child drawer
	w := v.width
	for _, d := range v.drawers {
		d.SetWidth(w)
	}
}

func span(drawers []Drawer, fn func(Drawer) bool) ([]Drawer, []Drawer) {
	g1 := make([]Drawer, 0)
	g2 := make([]Drawer, 0)
	for _, d := range drawers {
		if fn(d) {
			g1 = append(g1, d)
		} else {
			g2 = append(g2, d)
		}
	}
	return g1, g2
}
