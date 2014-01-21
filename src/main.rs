#[feature(globs)];

extern mod core = "sk_core";
extern mod ncurses;

use std::str;
use std::char;
use ncurses::*;
use core::gap_buffer::GapBuffer;

fn main()
{
  initscr();
  // Enter raw mode. In raw mode, normal line buffering and processing of interrupt, quit, suspend, and flow control keys are turned off; characters are presented to curses input functions one by one.
  raw();
  // In order to capture special keystrokes like Backspace, Delete and the four arrow keys by getch(), you need to call
  keypad(stdscr, true);
  // To suppress the automatic echoing of typed characters, you need to call
  noecho();

  loop {
    let ch = getch();
    let cha = char::from_u32(ch as u32).expect("Invalid char");
    if cha == 'q' {
      break;
    } else {
      printw(format!("{}", cha));
      refresh();
    }
  }

  endwin();
}

