package core

import (
	"testing"
)

func TestNewGapBuffer(t *testing.T) {
	buf := NewGapBuffer()
	if buf == nil {
		t.Error("buf should not be nil")
	}
}
