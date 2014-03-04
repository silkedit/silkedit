package config

import (
	"github.com/golang/glog"
	"github.com/BurntSushi/toml"
)

const (
	configFilePath = "sk.toml"
)

type Config struct {
	DefaultLineSeparator string `toml:"default_line_separator"`
}

var Conf Config

func Load() {
	if _, err := toml.DecodeFile(configFilePath, &Conf); err != nil {
		glog.Errorf("Can't read the setting file: %v", configFilePath)
	}
}

func (c *Config) LineSeparator() string {
	glog.Infof("default_line_separator: %v", c.DefaultLineSeparator)
	if c.DefaultLineSeparator == "LF" {
		return "\n"
	} else {
		return "\r\n"
	}
}
