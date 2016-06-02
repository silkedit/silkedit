#pragma once

#include "CustomWidget.h"
#include "Filtering.h"

namespace Ui {
class GeneralConfigView;
}

class GeneralConfigView : public CustomWidget, public Filtering {
  Q_OBJECT

 public:
  explicit GeneralConfigView(QWidget* parent = 0);
  ~GeneralConfigView();

 private:
  Ui::GeneralConfigView* ui;
};
