#pragma once

#include <QDialog>

#include "core/macros.h"

class Dialog : public QDialog {
  Q_OBJECT
  DISABLE_COPY(Dialog)

 public:
  // redefined here (only name is different) because Qt doesn't allow Q_ENUM(QDialog::DialogCode) in
  // sub class
  enum DialogCode { Rejected, Accepted };
  Q_ENUM(DialogCode)

  Q_INVOKABLE Dialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~Dialog();
  DEFAULT_MOVE(Dialog)

 public slots:
  void resize(int w, int h);
  void setGeometry(int x, int y, int w, int h);
  void setLayout(QLayout* layout);
};
