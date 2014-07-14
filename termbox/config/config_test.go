package config

import (
	"testing"
	yaml "gopkg.in/yaml.v1"
)

func TestLineSeparator(t *testing.T) {
	data := `
default_line_separator: "LF"
`
	if err := yaml.Unmarshal([]byte(data), &Conf); err != nil {
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
