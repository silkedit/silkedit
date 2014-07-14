package command

type Command struct {
	Name string
	Func func()
}

var Commands = map[string] Command {
	"exit": Command{ Name: "exit", Func: func() {
		panic("exit")
	}},
}
