#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QObject>

#include "core/Theme.h"
using core::Theme;

class TextEditView;

class LineNumberArea : public QWidget {
 public:
  static const int PADDING_RIGHT = 5;
  explicit LineNumberArea(TextEditView* editor);
  QSize sizeHint() const override;
  void setTheme(Theme* theme);

  QColor lineNumberColor() const;
  void setLineNumberColor(QColor color);
  QColor backgroundColor() const;
  void setBackgroundColor(QColor color);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  TextEditView* m_codeEditor;
  QColor m_lineNumberColor;
  QColor m_backgroundColor;
};

#endif  // LINENUMBERAREA_H
