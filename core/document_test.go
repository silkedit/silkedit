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
		t.Error("Len() should be 1")
	}
	if gb.Get(0) != 'a' {
		t.Error("char at 0 should be 'a'")
	}
}

func TestDelete(t *testing.T) {
	gb := NewGapBuffer()
	gb.Insert(0, 'a')
	gb.Insert(1, 'b')
	gb.Insert(2, 'c')
	if gb.Len() != 3 {
		t.Error("length should be 3")
	}

	// when
	gb.Delete(0)

	// then
	if gb.Len() != 2 {
		t.Error("length should be 2")
	}

	if gb.Get(0) != 'b' {
		t.Error("char at 0 should be 'b'")
	}
	if gb.Get(1) != 'c' {
		t.Error("char at 1 should be 'c'")
	}
}

func TestDeleteEmpty(t *testing.T) {
	gb := NewGapBuffer()

	// when
	gb.Delete(0)

	// then
	if gb.Len() != 0 {
		t.Error("length should be 0")
	}
}

func TestGrowBuffer(t *testing.T) {
	gb := NewGapBuffer()

	// when
	for i := uint(0); i < INITIAL_GAP_SIZE; i++ {
		gb.Insert(i, 'a')
	}
	gb.Insert(INITIAL_GAP_SIZE, 'b')

	// then
	if gb.Len() != INITIAL_GAP_SIZE+1 {
		t.Error("length should be 0")
	}
	if gb.Get(INITIAL_GAP_SIZE) != 'b' {
		t.Errorf("char at %v should be 'b', actual: %v", INITIAL_GAP_SIZE, gb.Get(INITIAL_GAP_SIZE))
	}
}

func TestForEach(t *testing.T) {
	gb := NewGapBuffer()
	gb.Insert(0, 'a')
	gb.Insert(1, 'b')
	gb.Insert(2, 'c')
	gb.Delete(1)

	// when
	var ss string
	gb.ForEach(func(i int, b byte) {
		ss += string(b)
	})

	// then
	if ss != "ac" {
		t.Errorf("expected: ac, actual: %x", ss)
	}
}

func TestSubscribe(t *testing.T) {
	gb := NewGapBuffer()
	var called = 0
	gb.Subscribe(func() {
		called++
	})
	gb.Subscribe(func() {
		called++
	})

	// when
	gb.Insert(0, 'a')
	gb.Insert(1, 'b')

	// then
	if called != 4 {
		t.Errorf("expected: 4, actual: %v", called)
	}
}
