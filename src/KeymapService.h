#pragma once

#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "CommandEvent.h"
#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QKeySequence;
class QKeyEvent;
class QString;

class KeymapService : public Singleton<KeymapService> {
  DISABLE_COPY_AND_MOVE(KeymapService)

 public:
  ~KeymapService() = default;

  void load(const QString& filename = "keymap.yml");
  bool dispatch(QKeyEvent* ev, int repeat = 1);

 private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  std::unordered_map<QKeySequence, CommandEvent> m_keymaps;
};
