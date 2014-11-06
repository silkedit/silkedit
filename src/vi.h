#pragma once

#define USE_EVENT_FILTER 1

enum Mode {
  CMD = 0,
  INSERT,
  CMDLINE,
};

namespace ViMoveOperation {
enum {
  FirstNonBlankChar = 100,  // ^
  LastChar,                 // $
  NextLine,                 // Enter, +
  PrevLine,                 // -
};
}
