#pragma once

#include <QStatusBar>

#include "macros.h"
#include "UniqueObject.h"

class TextEditView;
class LanguageComboBox;
class EncodingComboBox;
class Window;
struct Language;

class StatusBar : public QStatusBar, public UniqueObject<StatusBar> {
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  explicit StatusBar(Window* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void setCurrentLanguage(Language* lang);
  void setCurrentEncoding(const QString& encoding);

signals:
  void languageChanged(const QString& scopeName);

 public slots:
  void onActiveTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);
  void setActiveTextEditViewLanguage();
  void setLanguage(const QString& scope);

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
};
