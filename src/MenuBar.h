#pragma once

#include <QMenuBar>
#include <QAction>

#include "core/macros.h"

namespace core {
class Theme;
}

class MenuBar : public QMenuBar {
  Q_OBJECT
  DISABLE_COPY(MenuBar)

 public:
  static void init();
  static MenuBar* globalMenuBar() { return s_globalMenuBar; }

  MenuBar(QWidget* parent = nullptr);
  ~MenuBar() = default;
  DEFAULT_MOVE(MenuBar)

 private:
  static MenuBar* s_globalMenuBar;

  void themeActionTriggered(QAction* action);
  void showAboutDialog();
};

class ThemeMenu : public QMenu {
  Q_OBJECT
  DISABLE_COPY(ThemeMenu)

 public:
  ThemeMenu(const QString& title, QWidget* parent = nullptr);
  ~ThemeMenu() = default;
  DEFAULT_MOVE(ThemeMenu)

 private slots:
  void themeChanged(core::Theme* theme);
};

class ThemeAction : public QAction {
  Q_OBJECT
  DISABLE_COPY(ThemeAction)

 public:
  ThemeAction(const QString& text, QObject* parent);
  ~ThemeAction() = default;
  DEFAULT_MOVE(ThemeAction)
};
