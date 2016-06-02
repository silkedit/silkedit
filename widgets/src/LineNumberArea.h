#pragma once

#include "CustomWidget.h"

class TextEdit;
namespace core {
class Theme;
}

class LineNumberArea : public CustomWidget {
 public:
  static const int PADDING_RIGHT = 5;
  explicit LineNumberArea(TextEdit* editor);
  QSize sizeHint() const override;

  QColor lineNumberColor() const;
  void setLineNumberColor(QColor color);
  QColor backgroundColor() const;
  void setBackgroundColor(QColor color);
  QColor currentLineBackgroundColor() const { return m_currentLineBackgroundColor; }
  void setCurrentLineBackgroundColor(QColor color) { m_currentLineBackgroundColor = color; }

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  TextEdit* m_codeEditor;
  QColor m_lineNumberColor;
  QColor m_backgroundColor;
  QColor m_currentLineBackgroundColor;

  void setTheme(core::Theme* theme);
};
