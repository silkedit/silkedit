#pragma once

#include "CustomWidget.h"

namespace Ui {
class KeymapConfigView;
}

class KeymapConfigView : public CustomWidget {
  Q_OBJECT

 public:
  explicit KeymapConfigView(QWidget* parent = 0);
  ~KeymapConfigView();

 private:
  Ui::KeymapConfigView* ui;
};
