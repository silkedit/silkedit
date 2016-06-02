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

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  void setTheme(core::Theme* theme);
  TextEdit* m_codeEditor;
  QColor m_lineNumberColor;
  QColor m_backgroundColor;
};
