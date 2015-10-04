#include "ToolBar.h"

ToolBar::ToolBar(const QString& objectName, const QString& title, QWidget* parent)
    : QToolBar(title, parent) {
  setObjectName(objectName);
}
