package core

type GapBuffer struct {
	buffer    []byte
	gapOffset uint
	gapSize   uint
}

const INITIAL_GAP_SIZE = 128

func NewGapBuffer() *GapBuffer {
	return &GapBuffer{
		buffer:    make([]byte, INITIAL_GAP_SIZE),
		gapOffset: 0,
		gapSize:   INITIAL_GAP_SIZE,
	}
}

func (b *GapBuffer) confirmGap(newGapOffset uint) {
	if b.gapSize == 0 {
		b.gapOffset = uint(len(b.buffer))
		b.buffer = make([]byte, len(b.buffer)+INITIAL_GAP_SIZE)
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

func (gb *GapBuffer) Insert(offset uint, by byte) {
	gb.confirmGap(offset)
	gb.buffer[offset] = by
	gb.gapOffset++
	gb.gapSize--
}

func (gb *GapBuffer) Delete(offset uint) {
	if gb.Len() == 0 {
		return
	}

	gb.confirmGap(offset + 1)
	gb.gapOffset--
	gb.gapSize++
}

func (gb *GapBuffer) Len() uint {
	return uint(len(gb.buffer)) - gb.gapSize
}

func (gb *GapBuffer) Get(index uint) byte {
	var i uint = 0
	if index < gb.gapOffset {
		i = index
	} else {
		i = gb.gapSize + index
	}
	return gb.buffer[i]
}
