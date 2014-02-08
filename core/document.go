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
