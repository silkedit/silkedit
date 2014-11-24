#pragma once

#include <unordered_map>
#include <unordered_set>
#include <yaml-cpp/yaml.h>
#include <boost/optional.hpp>
#include <QObject>

#include "IKeyEventFilter.h"
#include "CommandEvent.h"
#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QKeySequence;
class QKeyEvent;
class QString;

class KeymapService : public Singleton<KeymapService>, public IKeyEventFilter {
  DISABLE_COPY_AND_MOVE(KeymapService)

 public:
  ~KeymapService() = default;

  void load(const QString& filename = "keymap.yml");
  boost::optional<QKeySequence> findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  bool dispatch(QKeyEvent* ev, int repeat = 1);

 private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  void add(const QKeySequence& key, CommandEvent cmdEvent);
  void clear();

  std::unordered_map<QKeySequence, CommandEvent> m_keymaps;
  std::unordered_map<QString, QKeySequence> m_cmdShortcuts;
};

class KeyHandler: public QObject, public Singleton<KeyHandler> {
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
