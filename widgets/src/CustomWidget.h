#pragma once

#include <QWidget>

// Base class of custom QWidget
// Custom widget which inherits QWidget directly ignores its stylesheet
class CustomWidget : public QWidget {
  Q_OBJECT
 public:
  explicit CustomWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  ~CustomWidget() = default;

 protected:
  virtual void paintEvent(QPaintEvent*);
};
