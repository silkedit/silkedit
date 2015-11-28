#pragma once

#include <QWidget>

class TextEditView;
namespace core {
class Theme;
}

class LineNumberArea : public QWidget {
 public:
  static const int PADDING_RIGHT = 5;
  explicit LineNumberArea(TextEditView* editor);
  QSize sizeHint() const override;

  QColor lineNumberColor() const;
  void setLineNumberColor(QColor color);
  QColor backgroundColor() const;
  void setBackgroundColor(QColor color);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  void setTheme(core::Theme* theme);
  TextEditView* m_codeEditor;
  QColor m_lineNumberColor;
  QColor m_backgroundColor;
};
