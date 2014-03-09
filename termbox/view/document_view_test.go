package view

import (
	"testing"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/suite"
)

type TestDrawer struct{}

func (d TestDrawer) DrawCursor(v *DocumentView) {

}

func (d TestDrawer) DrawDoc(v *DocumentView) {

}

type TestSuite struct {
	suite.Suite
}

func (suite *TestSuite) SetupTest() {
	getDrawer = func() Drawer {
		return TestDrawer{}
	}
}

func TestTestSuite(t *testing.T) {
	suite.Run(t, new(TestSuite))
}

func (suite *TestSuite) TestCursorPos() {
	v := NewDocumentView()
	x, y := v.CursorPos()
	assert.True(suite.T(), x == 0 && y == 0, "initial cursor pos should (0, 0)")
}

func (suite *TestSuite) TestDeleteCRLF() {
	v := NewDocumentView()
	v.InsertString("\r\n")
	v.Delete()
	line, column := v.CursorPos()
	assert.True(suite.T(), line == 0 && column == 0, "\r\n should be deleted at a time")
}
