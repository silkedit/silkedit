#pragma once

#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <boost/optional.hpp>

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
  boost::optional<QKeySequence> findShortcut(QString cmdName);

 public slots:
  bool dispatch(QKeyEvent* ev, int repeat = 1);

 private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  void add(const QKeySequence& key, CommandEvent cmdEvent);
  void clear();

  std::unordered_map<QKeySequence, CommandEvent> m_keymaps;
  std::unordered_map<QString, QKeySequence> m_cmdShortcuts;
};
