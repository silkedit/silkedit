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
  static void Init(v8::Local<v8::Object> exports);

  ~KeymapManager() = default;

  void loadUserKeymap();
  QKeySequence findShortcut(QString cmdName);
  bool keyEventFilter(QKeyEvent* event);
  const std::unordered_multimap<QKeySequence, CommandEvent>& keymaps() { return m_keymaps; }

 signals:
  void shortcutUpdated(const QString& cmdName, const QKeySequence& key);
  void keymapUpdated();

 private:
  static void Dispatch(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void _AssignJSKeyEventFilter(const v8::FunctionCallbackInfo<v8::Value>& args);

  friend class core::Singleton<KeymapManager>;
  KeymapManager();

  void add(const QKeySequence& key, CommandEvent cmdEvent);
  bool runJSKeyEventFilter(QKeyEvent* event);
  bool dispatch(QKeyEvent* ev, int repeat = 1);
  void load(const QString& filename, const QString& source);

  // internal (only used in initialization in JS side)
  void _assignJSKeyEventFilter(core::FunctionInfo info);

  // use multimap to store multiple keymaps that have same key combination but with different
  // condition
  std::unordered_multimap<QKeySequence, CommandEvent> m_keymaps;
  std::unordered_map<QString, Keymap> m_cmdKeymapHash;
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
