#pragma once

#include <QStatusBar>

#include "macros.h"

class TextEditView;
class LanguageComboBox;
class MainWindow;

class StatusBar: public QStatusBar {
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  StatusBar(MainWindow* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  public slots:
  void onActiveTextEditViewChanged(TextEditView* editView);

private:
  LanguageComboBox* m_langComboBox;
};
