package core

import (
	"testing"
)

func TestNewGapBuffer(t *testing.T) {
	// when
	gb := newGapBuffer()

	// then
	if gb == nil {
		t.Error("buf should not be nil")
	}
	if gb.Len() != 0 {
		t.Error("Len should be 0")
	}
}

func TestInsert(t *testing.T) {
	gb := newGapBuffer()
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
	gb := newGapBuffer()
	gb.Insert(0, 'あ')
	gb.Insert(1, 'い')
	gb.Insert(2, 'c')
	if gb.Len() != 3 {
		t.Errorf("expected: %v, actual: %v", 3, gb.Len())
	}

	// when
	isSuccess := gb.Delete(1)
	if isSuccess != true {
		t.Errorf("deletedSize should be true, actual: %v", isSuccess)
	}

	// then
	if gb.Len() != 2 {
		t.Errorf("expected: %v, actual: %v", 2, gb.Len())
	}

	if gb.Get(0) != 'い' {
		t.Errorf("expected char at 0: %v, actual: %v", 'い', gb.Get(0))
	}
	if gb.Get(1) != 'c' {
		t.Errorf("expected char at 1: %v, actual: %v", 'c', gb.Get(1))
	}
}

func TestDeleteEmpty(t *testing.T) {
	gb := newGapBuffer()

	// when
	gb.Delete(0)

	// then
	if gb.Len() != 0 {
		t.Error("length should be 0")
	}
}

func TestGrowBuffer(t *testing.T) {
	gb := newGapBuffer()

	// when
	for i := 0; i < INITIAL_GAP_SIZE; i++ {
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
	gb := newGapBuffer()
	gb.Insert(0, 'a')
	gb.Insert(1, 'b')
	gb.Insert(2, 'c')
	gb.Delete(2)

	// when
	var ss string
	gb.ForEach(func(r rune) {
		ss += string(r)
	})

	// then
	if ss != "ac" {
		t.Errorf("expected: ac, actual: %x", ss)
	}
}

func TestSubscribe(t *testing.T) {
	gb := newGapBuffer()
	var called = 0
	gb.Subscribe(func(int, interface{}) {
		called++
	})
	gb.Subscribe(func(int, interface{}) {
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

func TestDocument(t *testing.T) {
	// then
	gb := NewDocument()

	// when
	if gb == nil {
		t.Error("buf should not be nil")
	}
	if gb.Len() != 0 {
		t.Error("Len should be 0")
	}
}
