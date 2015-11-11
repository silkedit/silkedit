#pragma once

#include <QKeySequence>

#include "CommandEvent.h"
#include "core/macros.h"

struct Keymap {
  QKeySequence key;
  CommandEvent cmd;
};
