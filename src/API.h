#pragma once

#include <string>
#include <QList>

#include "macros.h"

class TextEditView;
class MainWindow;
class TabView;
class TabViewGroup;

class API {
  DISABLE_COPY_AND_MOVE(API)

 public:
  static TextEditView* activeEditView();
  static TabView* activeTabView(bool createIfNull = false);
  static TabViewGroup* activeTabViewGroup();
  static MainWindow* activeWindow();
  static QList<MainWindow*> windows();
  static void hideActiveFindReplacePanel();
  static void showDialog(const QString& msg);
  static void loadMenu(const std::string& ymlPath);

 private:
  API() = delete;
  ~API() = delete;
};
