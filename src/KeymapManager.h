#pragma once

#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <unordered_set>
#include <QObject>

#include "core/IKeyEventFilter.h"
#include "CommandEvent.h"
#include "core/macros.h"
#include "core/Singleton.h"
#include "core/stlSpecialization.h"

class QKeySequence;
class QKeyEvent;
class QString;

class KeymapManager : public core::Singleton<KeymapManager>, public core::IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(KeymapManager)

 public:
  ~KeymapManager() = default;

  void load();
  QKeySequence findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  bool dispatch(QKeyEvent* ev, int repeat = 1);

 private:
  friend class core::Singleton<KeymapManager>;
  KeymapManager() = default;

  void add(const QKeySequence& key, CommandEvent cmdEvent);
  void clear();
  void load(const QString& filename);
  void handleImports(const YAML::Node& node);
  void handleKeymap(const std::shared_ptr<Context>& context, const YAML::Node& node);

  // use multimap to store multiple keymaps that have same key combination but with different
  // context
  std::unordered_multimap<QKeySequence, CommandEvent> m_keymaps;
  std::unordered_map<QString, QKeySequence> m_cmdShortcuts;
  QString m_partiallyMatchedKeyString;
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
