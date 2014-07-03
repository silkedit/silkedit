package key

import (
	"bitbucket.org/shinichy/sk/termbox/view"
	"bitbucket.org/shinichy/sk/termbox/config"
	termbox "github.com/nsf/termbox-go"
)

func DispatchKey(v *view.DocumentView, ev termbox.Event) {
	switch ev.Key {
	case termbox.KeyEsc:
		panic("quit")
	case termbox.KeyBackspace, termbox.KeyBackspace2:
		v.Delete()
	case termbox.KeySpace:
		v.Insert(' ')
	case termbox.KeyEnter:
		v.InsertString(config.Conf.LineSeparator())
	default:
		if ev.Ch != 0 {
			v.Insert(ev.Ch)
		}
	}
}
