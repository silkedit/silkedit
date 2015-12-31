#pragma once

#include <QApplication>

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
  static TextEditView* activeTextEditView();
  static TabView* activeTabView();
  static TabViewGroup* activeTabViewGroup();
  static Window* activeWindow();
  static void restart();

  SilkApp(int& argc, char** argv);
  ~SilkApp() = default;

  void setupTranslator(const QString& locale);
  bool eventFilter(QObject *, QEvent *event) override;

protected:
  bool event(QEvent*) override;

 private:
  static SilkApp* s_app;

  QTranslator* m_translator;
  QTranslator* m_qtTranslator;
};
