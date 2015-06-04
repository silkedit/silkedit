#pragma once

#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <unordered_set>
#include <QObject>

#include "IKeyEventFilter.h"
#include "CommandEvent.h"
#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QKeySequence;
class QKeyEvent;
class QString;

class KeymapManager : public Singleton<KeymapManager>, public IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(KeymapManager)

 public:
  ~KeymapManager() = default;

  void load();
  QKeySequence findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  bool dispatch(QKeyEvent* ev, int repeat = 1);

 private:
  friend class Singleton<KeymapManager>;
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

class KeyHandler : public QObject, public Singleton<KeyHandler> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(KeyHandler)
 public:
  ~KeyHandler() = default;

  void registerKeyEventFilter(IKeyEventFilter* filter);
  void unregisterKeyEventFilter(IKeyEventFilter* filter);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  friend class Singleton<KeyHandler>;
  KeyHandler();

  std::unordered_set<IKeyEventFilter*> m_keyEventFilters;
};
