#pragma once

#include <QWidget>

namespace Ui {
class GeneralConfigView;
}

class GeneralConfigView : public QWidget {
  Q_OBJECT

 public:
  explicit GeneralConfigView(QWidget* parent = 0);
  ~GeneralConfigView();

 private:
  Ui::GeneralConfigView* ui;
};
