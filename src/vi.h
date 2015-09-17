#pragma once

#define USE_EVENT_FILTER 1

// todo: Move these enum to TextEditView

enum class Mode {
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
