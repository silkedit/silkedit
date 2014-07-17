package core

import (
	log "github.com/cihub/seelog"
	"unicode/utf8"
)

type Document interface {
IObservable
	Insert(int, rune) bool
	Delete(int) bool
	Len() int
	Get(int) rune
	Iterator() func() (rune, bool)
}

type GapBuffer struct {
	*Observable
	buffer      []byte
	gapOffset   int
	gapSize     int
}

const INITIAL_GAP_SIZE = 128

func NewDocument() Document {
	return newGapBuffer()
}

func newGapBuffer() *GapBuffer {
	return &GapBuffer{
		Observable:  NewObservable(),
		buffer:      make([]byte, INITIAL_GAP_SIZE),
		gapOffset:   0,
		gapSize:     INITIAL_GAP_SIZE,
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
	gb.CallSubscribers(DOCUMENT_INSERT, nil)
	return true
}

// Delete a character before cursor pos.
func (gb *GapBuffer) Delete(pos int) bool {
	if gb.Len() == 0 || pos <= 0 {
		log.Warnf("Can't delete at cursor pos: %v. Buffer length: %v", pos, gb.Len())
		return false
	}

	bytePos := gb.toBytePos(pos - 1)
	_, size := utf8.DecodeRune(gb.buffer[bytePos:])
	for i := size - 1; i >= 0; i-- {
		gb.deleteByte(bytePos + i)
	}

	gb.CallSubscribers(DOCUMENT_DELETE, nil)
	return true
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
	if pos < 0 {
		log.Errorf("invalid pos: %v", pos)
		return 0
	}

	n := gb.toBytePos(pos)
	r, _ := utf8.DecodeRune(gb.buffer[n:])
	return r
}

func (gb *GapBuffer) Iterator() func() (rune, bool) {
	beforeGap := gb.buffer[:gb.gapOffset]
	gapEnd := gb.gapOffset + gb.gapSize
	afterGap := gb.buffer[gapEnd:]
	return func() (rune, bool) {
		if len(beforeGap) > 0 {
			r, size := utf8.DecodeRune(beforeGap)
			beforeGap = beforeGap[size:]
			return r, true
		}

		if len(afterGap) > 0 {
			r, size := utf8.DecodeRune(afterGap)
			afterGap = afterGap[size:]
			return r, true
		}

		return 0, false
	}
}
