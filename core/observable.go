package core

type IObservable interface {
	Subscribe(func(int, interface{}))
}

type Observable struct {
	subscribers []func(int, interface{})
}

func (o *Observable) Subscribe(f func(ev int, info interface{})) {
	o.subscribers = append(o.subscribers, f)
}

func NewObservable() *Observable {
	return &Observable{
		subscribers: make([]func(int, interface{}), 0),
	}
}

func (o *Observable) callSubscribers(ev int, info interface{}) {
	for _, f := range o.subscribers {
		f(ev, info)
	}
}

