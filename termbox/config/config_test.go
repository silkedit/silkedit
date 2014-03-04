package config

import (
	"testing"
	"github.com/BurntSushi/toml"
)

func TestLineSeparator(t *testing.T) {
	blob := `
default_line_separator = "LF"
`
	if _, err := toml.Decode(blob, &(Conf)); err != nil {
		t.Error("Can't read the setting")
	}

	newline := Conf.LineSeparator()
	if newline != "\n" {
		t.Errorf("expected: \n, actual: %v", newline)
	}

	Conf.DefaultLineSeparator = "CRLF"
	newline = Conf.LineSeparator()
	if newline != "\r\n" {
		t.Errorf("expected: \r\n, actual: %v", newline)
	}

}
