#pragma once

#include <QStatusBar>

#include "macros.h"
#include "UniqueObject.h"

class TextEditView;
class LanguageComboBox;
class EncodingComboBox;
class Window;
struct Language;
class Encoding;

class StatusBar : public QStatusBar, public UniqueObject<StatusBar> {
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  explicit StatusBar(Window* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void onActiveTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);
  void setLanguage(const QString& scope);
  void setActiveTextEditViewLanguage();
  void setCurrentEncoding(const Encoding& encoding);

signals:
  void languageChanged(const QString& scopeName);

 protected:
  friend struct UniqueObject<StatusBar>;

  static void request(StatusBar* view,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(StatusBar* view, const QString& method, const msgpack::object& obj);

 private:
  LanguageComboBox* m_langComboBox;
  EncodingComboBox* m_encComboBox;

  void setCurrentLanguage(Language* lang);
};
