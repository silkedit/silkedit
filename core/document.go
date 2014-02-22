package core

import (
	"github.com/golang/glog"
	"unicode/utf8"
)

type Document interface {
	Insert(int, rune) bool
	Delete(int) bool
	Len() int
	Get(int) rune
	ForEach(func(rune))
	Subscribe(func())
}

type GapBuffer struct {
	buffer      []byte
	gapOffset   int
	gapSize     int
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

func (b *GapBuffer) confirmGap(newGapOffset int) {
	if b.gapSize == 0 {
		b.gapOffset = len(b.buffer)
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

func (gb *GapBuffer) insertByte(pos int, by byte) {
	gb.confirmGap(pos)
	gb.buffer[pos] = by
	gb.gapOffset++
	gb.gapSize--
}

func (gb *GapBuffer) deleteByte(pos int) {
	gb.confirmGap(pos + 1)
	gb.gapOffset--
	gb.gapSize++
}

// todo: change return type to bool

// Insert a rune at cursor pos.
// Returns the number of characters
func (gb *GapBuffer) Insert(pos int, r rune) bool {
	buf := make([]byte, 4)
	n := utf8.EncodeRune(buf, r)
	bytePos := gb.toBytePos(pos)
	for i := 0; i < n; i++ {
		gb.insertByte(bytePos+i, buf[i])
	}
	gb.callSubscribers()
	return true
}

// Delete a character before cursor pos.
func (gb *GapBuffer) Delete(pos int) bool {
	if gb.Len() == 0 || pos <= 0 {
		glog.Warningf("Can't delete at cursor pos: %v. Buffer length: %v", pos, gb.Len())
		return false
	}

	bytePos := gb.toBytePos(pos - 1)
	r, size := utf8.DecodeRune(gb.buffer[bytePos:])
	glog.Infof("Delete: %c, bytes: %v, pos: %v", r, size, pos)
	for i := size - 1; i >= 0; i-- {
		gb.deleteByte(bytePos + i)
	}

	gb.callSubscribers()
	return true
}

func (gb *GapBuffer) callSubscribers() {
	for _, f := range gb.subscribers {
		f()
	}
}

func (gb *GapBuffer) Len() int {
	beforeGap := gb.buffer[:gb.gapOffset]
	l := utf8.RuneCount(beforeGap)
	gapEnd := gb.gapOffset + gb.gapSize
	afterGap := gb.buffer[gapEnd:]
	l += utf8.RuneCount(afterGap)
	return l
}

func (gb *GapBuffer) toBytePos(pos int) int {
	n := 0
	for i := 0; i < pos; i++ {
		if n == gb.gapOffset && (n+gb.gapSize) != len(gb.buffer) {
			n += gb.gapSize
		}
		_, size := utf8.DecodeRune(gb.buffer[n:])
		n += size
	}
	if n == gb.gapOffset && (n+gb.gapSize) != len(gb.buffer) {
		n += gb.gapSize
	}
	return n
}

// TODO: error handling
func (gb *GapBuffer) Get(pos int) rune {
	n := gb.toBytePos(pos)
	r, _ := utf8.DecodeRune(gb.buffer[n:])
	return r
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
