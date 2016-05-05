#include <QKeyEvent>

#include "LineEdit.h"

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent) {
  setClearButtonEnabled(true);
}

LineEdit::LineEdit(const QString& contents, QWidget* parent) : QLineEdit(contents, parent) {
  setClearButtonEnabled(true);
}

void LineEdit::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Return:
      if (event->modifiers() & Qt::ShiftModifier) {
        emit shiftReturnPressed();
        return;
      }
  }

  QLineEdit::keyPressEvent(event);
}

void LineEdit::focusInEvent(QFocusEvent* ev) {
  emit focusIn();
  QLineEdit::focusInEvent(ev);
}
