package core

import (
	"github.com/golang/glog"
	"unicode/utf8"
)

type Document interface {
	Insert(uint, rune) int
	Delete(uint)
	Len() uint
	Get(uint) byte
	ForEach(func(rune))
	Subscribe(func())
}

type GapBuffer struct {
	buffer      []byte
	gapOffset   uint
	gapSize     uint
	subscribers []func()
}

const INITIAL_GAP_SIZE = 128

func NewDocument() Document {
	return newGapBuffer()
}

func newGapBuffer() *GapBuffer {
	return &GapBuffer{
		buffer:      make([]byte, INITIAL_GAP_SIZE),
		gapOffset:   0,
		gapSize:     INITIAL_GAP_SIZE,
		subscribers: make([]func(), 0),
	}
}

func (b *GapBuffer) confirmGap(newGapOffset uint) {
	if b.gapSize == 0 {
		b.gapOffset = uint(len(b.buffer))
		newBuffer := make([]byte, len(b.buffer)+INITIAL_GAP_SIZE)
		copy(newBuffer, b.buffer)
		b.buffer = newBuffer
		b.gapSize = INITIAL_GAP_SIZE
	}

	if newGapOffset < b.gapOffset {
		len := b.gapOffset - newGapOffset
		copy(b.buffer[newGapOffset+b.gapSize:], b.buffer[newGapOffset:newGapOffset+len])
	} else {
		len := newGapOffset - b.gapOffset
		copy(b.buffer[b.gapOffset:], b.buffer[b.gapOffset+b.gapSize:b.gapOffset+b.gapSize+len])
	}
	b.gapOffset = newGapOffset
}

func (gb *GapBuffer) insertByte(offset uint, by byte) {
	gb.confirmGap(offset)
	gb.buffer[offset] = by
	gb.gapOffset++
	gb.gapSize--
}

func (gb *GapBuffer) Insert(offset uint, r rune) int {
	buf := make([]byte, 4)
	n := utf8.EncodeRune(buf, r)
	for i := 0; i < n; i++ {
		gb.insertByte(offset+uint(i), buf[i])
	}
	gb.callSubscribers()
	return n
}

func (gb *GapBuffer) Delete(offset uint) {
	if gb.Len() == 0 {
		return
	}

	gb.confirmGap(offset + 1)
	gb.gapOffset--
	gb.gapSize++

	gb.callSubscribers()
}

func (gb *GapBuffer) callSubscribers() {
	for _, f := range gb.subscribers {
		f()
	}
}

func (gb *GapBuffer) Len() uint {
	return uint(len(gb.buffer)) - gb.gapSize
}

// TODO: error handling
func (gb *GapBuffer) Get(index uint) byte {
	var i uint = 0
	if index < gb.gapOffset {
		i = index
	} else {
		i = gb.gapSize + index
	}
	return gb.buffer[i]
}

func (gb *GapBuffer) ForEach(f func(rune)) {
	beforeGap := gb.buffer[:gb.gapOffset]
	for len(beforeGap) > 0 {
		r, size := utf8.DecodeRune(beforeGap)
		glog.Infof("r: %v, size: %v", r, size)
		f(r)
		beforeGap = beforeGap[size:]
	}

	gapEnd := gb.gapOffset + gb.gapSize
	afterGap := gb.buffer[gapEnd:]
	for len(afterGap) > 0 {
		r, size := utf8.DecodeRune(afterGap)
		glog.Infof("r: %v, size: %v", r, size)
		f(r)
		afterGap = afterGap[size:]
	}
}

func (gb *GapBuffer) Subscribe(f func()) {
	gb.subscribers = append(gb.subscribers, f)
}
