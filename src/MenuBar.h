#pragma once

#include <QMenuBar>
#include <QAction>

#include "macros.h"

class MenuBar : public QMenuBar {
  DISABLE_COPY(MenuBar)

 public:
  MenuBar(QWidget* parent = nullptr);
  ~MenuBar() = default;
  DEFAULT_MOVE(MenuBar)
};

class ThemeAction : public QAction {
  Q_OBJECT
  DISABLE_COPY(ThemeAction)

 public:
  ThemeAction(const QString& text, QObject* parent);
  ~ThemeAction() = default;
  DEFAULT_MOVE(ThemeAction)

 private slots:
  void themeSelected();
};
