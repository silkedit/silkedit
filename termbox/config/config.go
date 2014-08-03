package config

import (
	log "github.com/cihub/seelog"
	"io/ioutil"
	"github.com/robertkrimen/otto"
)

const (
	configFilePath = "sk.js"
)

type Config struct {
	DefaultLineSeparator string "default_line_separator"
}

var Conf = Config{}

func Load() {
	contents, err := ioutil.ReadFile(configFilePath)
	if err != nil {
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	vm := otto.New()
	value, err := vm.Run(contents)
	if err != nil {
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	itf, err := value.Export()
	if err != nil{
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	config, ok := itf.(map[string]interface{})
	if ok {
		value, ok := config["default_line_separator"]
		if ok {
			var defaultLineSeparator, _ = value.(string)
			Conf.DefaultLineSeparator = defaultLineSeparator
		}
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
