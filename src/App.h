#pragma once

#include <QApplication>
#include <QSettings>

#include "core/macros.h"

class TabBar;
class TextEdit;
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
  static void saveSession();
  static void loadSession();

  App(int& argc, char** argv);
  ~App() = default;

  void setupTranslator(const QString& locale);
  bool isQuitting() { return m_isQuitting; }
  TabView* getActiveTabViewOrCreate();

 public slots:
  TextEdit* activeTextEdit();
  TabView* activeTabView();
  TabViewGroup* activeTabViewGroup();
  Window* activeWindow();
  void setActiveWindow(QWidget* act);
  QWidget* focusWidget();
  QWidget* activePopupWidget();
  void postEvent(QObject* receiver, QEvent* event, int priority = Qt::NormalEventPriority);

 protected:
  bool event(QEvent*) override;
  bool notify(QObject* receiver, QEvent* event) override;

 private:
  static App* s_app;
  static bool m_isCleanedUp;

  QTranslator* m_translator;
  QTranslator* m_qtTranslator;
  bool m_isQuitting;

  void cleanup();
};
