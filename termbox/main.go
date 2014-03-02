package main

import (
	"bitbucket.org/shinichy/sk/core"
	. "./api"
	"flag"
	"github.com/golang/glog"
	"github.com/nsf/termbox-go"
)

func main() {
	flag.Parse()
	glog.Info("Initialize termbox")
	err := termbox.Init()
	if err != nil {
		panic(err)
	}
	defer termbox.Close()
	termbox.SetInputMode(termbox.InputEsc | termbox.InputMouse)

	drawer := TermboxDrawer{}
	view := core.NewDocumentView(drawer)
mainloop:
	for {
		switch ev := termbox.PollEvent(); ev.Type {
		case termbox.EventKey:
			switch ev.Key {
			case termbox.KeyEsc:
				break mainloop
			case termbox.KeyBackspace:
			case termbox.KeyBackspace2:
				view.Delete()
			case termbox.KeySpace:
				view.Insert(' ')
			case termbox.KeyEnter:
				// todo: create a new line character configuration. #16
				view.Insert('\n')
			default:
				if ev.Ch != 0 {
					view.Insert(ev.Ch)
				}
			}
		case termbox.EventError:
			panic(ev.Err)
		}
	}

	glog.Flush()
}
