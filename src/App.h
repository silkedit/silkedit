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

public slots:
  TextEditView* activeTextEditView();
  TabView* activeTabView();
  TabViewGroup* activeTabViewGroup();
  Window* activeWindow();
  void setActiveWindow(QWidget* act);
  QWidget* focusWidget();

protected:
  bool event(QEvent*) override;
  bool notify(QObject * receiver, QEvent * event) override;

 private:
  static App* s_app;

  QTranslator* m_translator;
  QTranslator* m_qtTranslator;
};
