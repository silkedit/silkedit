#pragma once

#include <QWidget>
#include <QLineEdit>

#include "macros.h"

class LineEdit;

class FindReplaceView : public QWidget {
  Q_OBJECT
  DISABLE_COPY(FindReplaceView)

 public:
  explicit FindReplaceView(QWidget* parent);
  ~FindReplaceView() = default;
  DEFAULT_MOVE(FindReplaceView)

  protected:
    void showEvent(QShowEvent * event) override;

 private:
  LineEdit* m_lineEditForFind;

    void findNext();
    void findNextText(const QString& text);
};

class LineEdit : public QLineEdit {
  Q_OBJECT
  DISABLE_COPY(LineEdit)
 public:
  explicit LineEdit(QWidget* parent);
  ~LineEdit() = default;
  DEFAULT_MOVE(LineEdit)

  protected:
    void keyPressEvent(QKeyEvent * event) override;

  signals:
    void escapePressed();
};
