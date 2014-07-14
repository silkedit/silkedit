package config

import (
	"github.com/golang/glog"
	"io/ioutil"
	yaml "gopkg.in/yaml.v1"
)

const (
	configFilePath = "sk.yml"
)

type Config struct {
	DefaultLineSeparator string "default_line_separator"
}

var Conf Config

func Load() {
	contents, err := ioutil.ReadFile(configFilePath)
	if err != nil {
		glog.Errorf("Can't read the setting file: %v", err)
		return
	}

	if err := yaml.Unmarshal([]byte(contents), &Conf); err != nil {
		glog.Errorf("Failed to unmarshal, %v, %v", configFilePath, err)
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
