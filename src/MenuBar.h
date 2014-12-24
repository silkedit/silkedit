#pragma once

#include <QMenuBar>
#include <QAction>

#include "macros.h"

class MenuBar : public QMenuBar {
  Q_OBJECT
  DISABLE_COPY(MenuBar)

 public:
  MenuBar(QWidget* parent = nullptr);
  ~MenuBar() = default;
  DEFAULT_MOVE(MenuBar)

  private slots:
    void themeActionTriggered(QAction* action);
};

class ThemeMenu : public QMenu {
  Q_OBJECT
  DISABLE_COPY(ThemeMenu)

 public:
  ThemeMenu(const QString& title, QWidget* parent = nullptr);
  ~ThemeMenu() = default;
  DEFAULT_MOVE(ThemeMenu)

 private slots:
  void themeChanged(Theme* theme);
};

class ThemeAction : public QAction {
  Q_OBJECT
  DISABLE_COPY(ThemeAction)

 public:
  ThemeAction(const QString& text, QObject* parent);
  ~ThemeAction() = default;
  DEFAULT_MOVE(ThemeAction)
};
