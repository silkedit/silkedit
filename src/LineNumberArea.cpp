#include "LineNumberArea.h"
#include "TextEditView.h"
#include "core/Theme.h"
#include "core/Config.h"

using core::ColorSettings;
using core::Theme;
using core::Config;

LineNumberArea::LineNumberArea(TextEditView* editor) : QWidget(editor), m_codeEditor(editor) {
  connect(&Config::singleton(), &Config::themeChanged, this, &LineNumberArea::setTheme);
  // Set default values
  setTheme(Config::singleton().theme());
}

QSize LineNumberArea::sizeHint() const {
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::setTheme(Theme* theme) {
  qDebug("LineNumberArea theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }
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
