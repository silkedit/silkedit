package main

import (
	"flag"
	"os"
	"bitbucket.org/shinichy/sk/termbox/view"
	"bitbucket.org/shinichy/sk/termbox/config"
	"github.com/golang/glog"
	termbox "github.com/nsf/termbox-go"
)

func main() {
	flag.Parse()
	glog.Info("Loading settings")
	config.Load()

	glog.Info("Loading key maps")
	config.LoadKeyMap()

	glog.Info("Initialize termbox")
	err := termbox.Init()
	if err != nil {
		panic(err)
	}

	defer func() {
		termbox.Close()
		glog.Flush()
		os.Exit(0)
	}()

	termbox.SetInputMode(termbox.InputEsc | termbox.InputMouse)

	// construct views
	root := view.NewStackView()
	v := view.NewDocumentView()
	statusView := view.NewStatusView()
	width, height := termbox.Size()
	root.SetWidth(width)
	root.SetHeight(height)
	root.Add(v)
	root.Add(statusView)
	root.Draw(0, 0)
	termbox.Flush()

	for {
		// event handling
		switch ev := termbox.PollEvent(); ev.Type {
		case termbox.EventKey:
			config.DispatchKey(v, ev)
		case termbox.EventError:
			panic(ev.Err)
		}

		root.Draw(0, 0)
		termbox.Flush()
	}
}
