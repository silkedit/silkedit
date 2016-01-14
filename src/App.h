#pragma once

#include <QApplication>

#include "core/macros.h"

class TabBar;
class TextEditView;
class Window;
class TabView;
class TabViewGroup;

class App : public QApplication {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(App)

 public:
  static App* instance() { return s_app; }
  static TabBar* tabBarAt(int x, int y);
  static void restart();

  App(int& argc, char** argv);
  ~App() = default;

  void setupTranslator(const QString& locale);
  bool eventFilter(QObject *, QEvent *event) override;

public slots:
  TextEditView* activeTextEditView();
  TabView* activeTabView();
  TabViewGroup* activeTabViewGroup();
  Window* activeWindow();

protected:
  bool event(QEvent*) override;

 private:
  static App* s_app;

  QTranslator* m_translator;
  QTranslator* m_qtTranslator;
};
