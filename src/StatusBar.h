#pragma once

#include <QStatusBar>

#include "macros.h"

class TextEditView;
class LanguageComboBox;
class MainWindow;
struct Language;

class StatusBar : public QStatusBar {
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  StatusBar(MainWindow* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void setCurrentLanguage(Language* lang);

signals:
  void languageChanged(const QString& scopeName);

 public slots:
  void onActiveTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);
  void setActiveTextEditViewLanguage();
  void setLanguage(const QString& scope);

 private:
  LanguageComboBox* m_langComboBox;
};
