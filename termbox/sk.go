package main

import (
	"flag"
	"os"
	"bitbucket.org/shinichy/sk/termbox/view"
	"bitbucket.org/shinichy/sk/termbox/config"
	termbox "github.com/nsf/termbox-go"
	log "github.com/cihub/seelog"
)

func main() {
	if logger, err := log.LoggerFromConfigAsFile("seelog.xml"); err != nil {
		panic(err)
	} else {
		log.ReplaceLogger(logger)
	}

	flag.Parse()
	log.Info("Loading settings")
	config.Load()

	log.Info("Loading key maps")
	config.LoadKeyMap()

	log.Info("Initialize termbox")
	if err := termbox.Init(); err != nil {
		panic(err)
	}

	defer func() {
		log.Info("shutting down")
		log.Info("")
		log.Flush()
		termbox.Close()
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
