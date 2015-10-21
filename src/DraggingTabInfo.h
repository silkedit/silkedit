#pragma once

#include <QString>

#include "core/macros.h"

class QWidget;

class DraggingTabInfo {
  DISABLE_COPY_AND_MOVE(DraggingTabInfo)

 public:
  static QWidget* widget() { return s_widget; }
  static void setWidget(QWidget* w) { s_widget = w; }

  static QString tabText() { return s_tabText; }
  static void setTabText(const QString& text) { s_tabText = text; }

 private:
  DraggingTabInfo() = default;
  ~DraggingTabInfo() = default;

  static QWidget* s_widget;
  static QString s_tabText;
};
