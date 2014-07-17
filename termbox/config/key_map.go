package config

import (
	yaml "gopkg.in/yaml.v1"
	"io/ioutil"
	log "github.com/cihub/seelog"
	termbox "github.com/nsf/termbox-go"
	"bitbucket.org/shinichy/sk/termbox/view"
	. "bitbucket.org/shinichy/sk/termbox/command"
	"fmt"
)

const (
	keyMapFilePath = "key.yml"
)

type KeyMapDefinition struct {
	Shortcuts []struct{Key string; Cmd string}
}

type KeyIdentifier struct {
	key    termbox.Key       // one of Key* constants, invalid if 'Ch' is not 0
	ch     rune      // a unicode character
}

var shortcutDefinition KeyMapDefinition

var KeyCommandMap = make(map[KeyIdentifier]Command)

func LoadKeyMap() {
	contents, err := ioutil.ReadFile(keyMapFilePath)
	if err != nil {
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	if err := yaml.Unmarshal([]byte(contents), &shortcutDefinition); err != nil {
		log.Errorf("Failed to unmarshal. path: %v, %v", keyMapFilePath, err)
		return
	}

	count := 0
	for _, shortcut := range shortcutDefinition.Shortcuts {
		if id, err := convertKeyToEv(shortcut.Key); err == nil {
			if cmd, exists := Commands[shortcut.Cmd]; exists {
				KeyCommandMap[*id] = cmd
				count++
			} else {
				log.Warnf("Unknown cmd: %v", shortcut.Cmd)
			}
		} else {
			log.Warnf("Unknown key string: %v", shortcut.Key)
		}
	}

	log.Infof("%v key maps loaded", count)
}

func convertKeyToEv(key string) (*KeyIdentifier, error) {
	switch key {
		case "ctrl+q": return &KeyIdentifier{key: termbox.KeyCtrlQ, ch: 0}, nil
	}

	return nil, fmt.Errorf("%v not found", key)
}

func DispatchKey(v *view.DocumentView, ev termbox.Event) {
	log.Debugf("Event: %v", ev)

	id := KeyIdentifier{key: ev.Key, ch: ev.Ch}
	if cmd, exists := KeyCommandMap[id]; exists {
		cmd.Func()
	} else {
		switch ev.Key {
		case termbox.KeyBackspace, termbox.KeyBackspace2:
			v.Delete()
		case termbox.KeySpace:
			v.Insert(' ')
		case termbox.KeyEnter:
			v.InsertString(Conf.LineSeparator())
		default:
			if ev.Ch != 0 {
				v.Insert(ev.Ch)
			}
		}
	}
}
