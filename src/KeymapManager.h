#pragma once

#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <unordered_set>
#include <QObject>
#include <QFileSystemWatcher>

#include "CommandEvent.h"
#include "Keymap.h"
#include "core/IKeyEventFilter.h"
#include "core/macros.h"
#include "core/Singleton.h"
#include "core/stlSpecialization.h"

class QKeySequence;
class QKeyEvent;
class QString;

class KeymapManager : public QObject,
                      public core::Singleton<KeymapManager>,
                      public core::IKeyEventFilter {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(KeymapManager)

 public:
  ~KeymapManager() = default;

  void loadUserKeymap();
  void load(const QString& filename, const QString& source = "");
  QKeySequence findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  bool dispatch(QKeyEvent* ev, int repeat = 1);
  const std::unordered_multimap<QKeySequence, CommandEvent>& keymaps() { return m_keymaps; }

 signals:
  void shortcutUpdated(const QString& cmdName, const QKeySequence& key);
  void keymapUpdated();

 private:
  friend class core::Singleton<KeymapManager>;
  KeymapManager();

  void add(const QKeySequence& key, CommandEvent cmdEvent);

  // use multimap to store multiple keymaps that have same key combination but with different
  // condition
  std::unordered_multimap<QKeySequence, CommandEvent> m_keymaps;
  std::unordered_map<QString, Keymap> m_cmdKeymapHash;
  QString m_partiallyMatchedKeyString;
  void removeUserKeymap();
};

class TextEditViewKeyHandler : public QObject, public core::Singleton<TextEditViewKeyHandler> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(TextEditViewKeyHandler)
 public:
  ~TextEditViewKeyHandler() = default;

  void registerKeyEventFilter(core::IKeyEventFilter* filter);
  void unregisterKeyEventFilter(core::IKeyEventFilter* filter);
  bool dispatchKeyPressEvent(QKeyEvent* event);

 private:
  friend class core::Singleton<TextEditViewKeyHandler>;
  TextEditViewKeyHandler();

  std::unordered_set<core::IKeyEventFilter*> m_keyEventFilters;
};
