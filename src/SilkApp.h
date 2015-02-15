#pragma once

#include <QApplication>

#include "macros.h"

class TabBar;
class TextEditView;
class MainWindow;
class TabView;
class TabViewGroup;

class SilkApp : public QApplication {
  DISABLE_COPY_AND_MOVE(SilkApp)

 public:
  static TabBar* tabBarAt(int x, int y);
  static TextEditView* activeEditView();
  static TabView* activeTabView(bool createIfNull = false);
  static TabViewGroup* activeTabViewGroup();
  static MainWindow* activeWindow();

  SilkApp(int& argc, char** argv);
  ~SilkApp() = default;

  // accessor
  QString initialFile() { return m_initialFile; }

  bool event(QEvent*) override;

 private:
  QString m_initialFile;
};
