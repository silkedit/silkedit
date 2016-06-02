#include "LineNumberArea.h"
#include "TextEdit.h"
#include "core/Theme.h"
#include "core/Config.h"

using core::ColorSettings;
using core::Theme;
using core::Config;

LineNumberArea::LineNumberArea(TextEdit* editor) : CustomWidget(editor), m_codeEditor(editor) {
  connect(&Config::singleton(), &Config::themeChanged, this, &LineNumberArea::setTheme);
  // Set default values
  setTheme(Config::singleton().theme());
}

QSize LineNumberArea::sizeHint() const {
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::setTheme(Theme* theme) {
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  static const auto& backgroundKey = QStringLiteral("background");
  static const auto& foregroundKey = QStringLiteral("foreground");
  static const auto& selectionBackgroundKey = QStringLiteral("selectionBackground");

  if (theme->gutterSettings) {
    if (theme->gutterSettings->contains(backgroundKey)) {
      setBackgroundColor(theme->gutterSettings->value(backgroundKey));
    }

    if (theme->gutterSettings->contains(foregroundKey)) {
      setLineNumberColor(theme->gutterSettings->value(foregroundKey));
    }
  }

  if (theme->textEditSettings && theme->textEditSettings->contains(selectionBackgroundKey)) {
    setCurrentLineBackgroundColor(
        theme->textEditSettings->value(selectionBackgroundKey));
  }
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
  Q_ASSERT(m_codeEditor);
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
