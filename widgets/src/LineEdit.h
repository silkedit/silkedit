#pragma once

#include <QLineEdit>

#include "core/macros.h"

class LineEdit : public QLineEdit {
  Q_OBJECT
  DISABLE_COPY(LineEdit)
 public:
  Q_INVOKABLE explicit LineEdit(QWidget* parent = nullptr);
  Q_INVOKABLE explicit LineEdit(const QString& contents, QWidget* parent = nullptr);
  ~LineEdit() = default;
  DEFAULT_MOVE(LineEdit)

 public slots:
  bool hasAcceptableInput() const;

  void setValidator(const QValidator* v);
  const QValidator* validator() const;

 protected:
  void keyPressEvent(QKeyEvent* event) override;
  void focusInEvent(QFocusEvent* ev) override;

 signals:
  void shiftReturnPressed();
  void focusIn();
};
