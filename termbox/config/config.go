package config

import (
	log "github.com/cihub/seelog"
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
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	if err := yaml.Unmarshal([]byte(contents), &Conf); err != nil {
		log.Errorf("Failed to unmarshal, %v, %v", configFilePath, err)
	}
}

func (c *Config) LineSeparator() string {
	log.Infof("default_line_separator: %v", c.DefaultLineSeparator)
	if c.DefaultLineSeparator == "LF" {
		return "\n"
	} else {
		return "\r\n"
	}
}
