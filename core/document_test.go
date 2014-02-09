package core

import (
	"testing"
)

func TestNewGapBuffer(t *testing.T) {
	// when
	gb := NewGapBuffer()

	// then
	if gb == nil {
		t.Error("buf should not be nil")
	}
	if gb.Len() != 0 {
		t.Error("Len should be 0")
	}
}

func TestInsert(t *testing.T) {
	gb := NewGapBuffer()
	if gb.Len() != 0 {
		t.Error("Len should be 0")
	}

	// when
	gb.Insert(0, 'a')

	// then
	if gb.Len() != 1 {
		t.Error("buf should not be nil")
	}
	if gb.Get(0) != 'a' {
		t.Error("buf should not be nil")
	}
}
