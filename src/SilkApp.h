#pragma once

#include <QApplication>
#include <QNetworkAccessManager>

#include "core/macros.h"

class TabBar;
class TextEditView;
class Window;
class TabView;
class TabViewGroup;

class SilkApp : public QApplication {
  DISABLE_COPY_AND_MOVE(SilkApp)

 public:
  static TabBar* tabBarAt(int x, int y);
  static TextEditView* activeEditView();
  static TabView* activeTabView();
  static TabViewGroup* activeTabViewGroup();
  static Window* activeWindow();
  static QNetworkAccessManager* networkManager() { return s_manager; }

  SilkApp(int& argc, char** argv);
  ~SilkApp() = default;

  bool event(QEvent*) override;

 private:
  // One QNetworkAccessManager should be enough for the whole Qt application.
  static QNetworkAccessManager* s_manager;
};
