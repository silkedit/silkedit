package config

import (
	"io/ioutil"
	log "github.com/cihub/seelog"
	termbox "github.com/nsf/termbox-go"
	"bitbucket.org/shinichy/sk/termbox/view"
	. "bitbucket.org/shinichy/sk/termbox/command"
	"fmt"
	"github.com/robertkrimen/otto"
)

const (
	keyMapFilePath = "key.js"
)

type KeyIdentifier struct {
	key    termbox.Key       // one of Key* constants, invalid if 'Ch' is not 0
	ch     rune      // a unicode character
}

var KeyCommandMap = make(map[KeyIdentifier]Command)

func LoadKeyMap() {
	contents, err := ioutil.ReadFile(keyMapFilePath)
	if err != nil {
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	vm := otto.New()
	var value, errOtto = vm.Run(contents)
	if errOtto != nil {
		log.Errorf("Can't read the setting file: %v", errOtto)
		return
	}

	it, errExport := value.Export()
	if errExport != nil{
		log.Errorf("Can't read the setting file: %v", errExport)
		return
	}

	arr, ok := it.([]interface{})
	if !ok {
		log.Errorf("Can't read the setting file: %v", err)
		return
	}

	count := 0
	for i := 0; i < len(arr); i++ {
		shortcut, ok := arr[i].(map[string]interface{})
		if ok {
			key, keyExists := shortcut["key"]
			cmd, cmdExists := shortcut["command"]
			if keyExists && cmdExists {
				key, _ := key.(string)
				cmd, _ := cmd.(string)
				if id, err := convertKeyToEv(key); err == nil {
					if cmd, exists := Commands[cmd]; exists {
						KeyCommandMap[*id] = cmd
						count++
					} else {
						log.Warnf("Unknown cmd: %v", cmd)
					}
				} else {
					log.Warnf("Unknown key string: %v", key)
				}
			}

		} else {
			log.Warnf("pos %v is not map[string]string", i)
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
