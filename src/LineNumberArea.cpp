#include <QStyleOption>
#include <QPainter>
#include <QDebug>

#include "LineNumberArea.h"
#include "TextEditView.h"

using core::ColorSettings;

LineNumberArea::LineNumberArea(TextEditView* editor) : QWidget(editor), m_codeEditor(editor) {}

QSize LineNumberArea::sizeHint() const {
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::setTheme(Theme* theme) {
  if (theme->gutterSettings != nullptr) {
    ColorSettings* gutterSettings = theme->gutterSettings.get();
    if (gutterSettings->contains("background")) {
      setBackgroundColor(gutterSettings->value("background").name());
    }

    if (gutterSettings->contains("foreground")) {
      setLineNumberColor(gutterSettings->value("foreground").name());
    }
  }
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
  m_codeEditor->lineNumberAreaPaintEvent(event);
}

QColor LineNumberArea::lineNumberColor() const {
  return m_lineNumberColor;
}

void LineNumberArea::setLineNumberColor(QColor color) {
  m_lineNumberColor = color;
}

QColor LineNumberArea::backgroundColor() const {
  return m_backgroundColor;
}

void LineNumberArea::setBackgroundColor(QColor color) {
  m_backgroundColor = color;
}
