extern mod std;
extern mod termbox;

use std::io::Timer;
use tb = termbox;

fn main() {
	tb::init();
	tb::print(1, 1, tb::bold, tb::white, tb::black, ~"Hello, world!");
	tb::present();
	let mut timer = Timer::new().unwrap();
	timer.sleep(1000);
	tb::shutdown();
}
