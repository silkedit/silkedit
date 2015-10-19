#pragma once

#include <QStatusBar>

#include "core/macros.h"
#include "core/UniqueObject.h"

class TextEditView;
class LanguageComboBox;
class EncodingComboBox;
class LineSeparatorComboBox;
class QMainWindow;
namespace core {
struct Language;
class Encoding;
}

class StatusBar : public QStatusBar, public core::UniqueObject<StatusBar> {
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  explicit StatusBar(QMainWindow* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void onActiveTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);
  void setLanguage(const QString& scope);
  void setEncoding(const core::Encoding& encoding);
  void setLineSeparator(const QString& separator);
  void setActiveTextEditViewLanguage();
  void setActiveTextEditViewEncoding();
  void setActiveTextEditViewLineSeparator();

signals:
  void languageChanged(const QString& scopeName);

 protected:
  friend struct core::UniqueObject<StatusBar>;

  static void request(StatusBar* view,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(StatusBar* view, const QString& method, const msgpack::object& obj);

 private:
  LanguageComboBox* m_langComboBox;
  LineSeparatorComboBox* m_separatorComboBox;
  EncodingComboBox* m_encComboBox;

  void setCurrentLanguage(core::Language* lang);
};
