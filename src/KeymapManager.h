#pragma once

#include <v8.h>
#include <unordered_map>
#include <unordered_set>
#include <QObject>

#include "CommandEvent.h"
#include "Keymap.h"
#include "core/IKeyEventFilter.h"
#include "core/macros.h"
#include "core/Singleton.h"
#include "core/stlSpecialization.h"
#include "core/FunctionInfo.h"

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
  QKeySequence findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  const std::unordered_multimap<QKeySequence, CommandEvent>& keymaps() { return m_keymaps; }

public slots:
  bool dispatch(QKeyEvent* ev, int repeat = 1);
  void load(const QString& filename, const QString& source);

  // internal (only used in initialization in JS side)
  void _assignJSKeyEventFilter(core::FunctionInfo info);

 signals:
  void shortcutUpdated(const QString& cmdName, const QKeySequence& key);
  void keymapUpdated();

 private:
  friend class core::Singleton<KeymapManager>;
  KeymapManager();

  void add(const QKeySequence& key, CommandEvent cmdEvent);
  bool runJSKeyEventFilter(QKeyEvent* event);

  // use multimap to store multiple keymaps that have same key combination but with different
  // condition
  std::unordered_multimap<QKeySequence, CommandEvent> m_keymaps;

  // store shortcuts with same key but different condition
  // e.g.
  // - { key: 'ctrl+`', command: show_console, if: console_visible == false}
  // - { key: 'ctrl+`', command: hide_console, if: console_visible}
  //
  // must not have duplicate keymaps like this.
  // - { key: 'ctrl+`', command: show_console}
  // - { key: 'ctrl+`', command: hide_console}
  // In this case, lower priority's keymap is removed
  std::unordered_multimap<QString, Keymap> m_cmdKeymapHash;

  QString m_partiallyMatchedKeyString;
  std::unordered_map<QKeySequence, CommandEvent> m_emptyCmdKeymap;
  v8::UniquePersistent<v8::Function> m_jsKeyEventFilter;

  void removeKeymap();
  void removeShortcut(const QString& cmdName);
  void addShortcut(const QKeySequence& key, CommandEvent cmdEvent);
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
