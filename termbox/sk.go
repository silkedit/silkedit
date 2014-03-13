package main

import (
	"flag"
	"bitbucket.org/shinichy/sk/termbox/view"
	"bitbucket.org/shinichy/sk/termbox/config"
	"github.com/golang/glog"
	"github.com/nsf/termbox-go"
)

func main() {
	flag.Parse()
	glog.Info("Loading settings")
	config.Load()

	glog.Info("Initialize termbox")
	err := termbox.Init()
	if err != nil {
		panic(err)
	}
	defer termbox.Close()
	termbox.SetInputMode(termbox.InputEsc | termbox.InputMouse)

	// construct views
	root := view.NewStackView()
	v := view.NewDocumentView()
	width, height := termbox.Size()
	root.SetWidth(width)
	root.SetHeight(height)
	root.Add(v)
	root.Draw(0, 0)

mainloop:
	for {
		switch ev := termbox.PollEvent(); ev.Type {
		case termbox.EventKey:
			switch ev.Key {
			case termbox.KeyEsc:
				break mainloop
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
		case termbox.EventError:
			panic(ev.Err)
		}
	}

	glog.Flush()
}
