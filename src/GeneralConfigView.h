#pragma once

#include <QWidget>

#include "Filtering.h"

namespace Ui {
class GeneralConfigView;
}

class GeneralConfigView : public QWidget, public Filtering {
  Q_OBJECT

 public:
  explicit GeneralConfigView(QWidget* parent = 0);
  ~GeneralConfigView();

 private:
  Ui::GeneralConfigView* ui;
};
