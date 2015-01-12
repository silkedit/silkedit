#pragma once

#include <QList>

#include "macros.h"

class TextEditView;
class MainWindow;
class TabWidget;

class API {
  DISABLE_COPY_AND_MOVE(API)

 public:
  API() = default;
  ~API() = default;

  static TextEditView* activeEditView();
  static TabWidget* activeTabWidget();
  static MainWindow* activeWindow();
  static QList<MainWindow*> windows();
};
