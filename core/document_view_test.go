package core

import (
	"testing"
)

func TestCursorPos(t *testing.T) {
	v := NewDocumentView()
	if x, y := v.CursorPos(); x != 0 || y != 0 {
		t.Errorf("expected initial cursor pos (0, 0), actual: %v, %v", x, y)
	}
}
