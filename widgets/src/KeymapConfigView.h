#pragma once

#include <QWidget>

namespace Ui {
class KeymapConfigView;
}

class KeymapConfigView : public QWidget {
  Q_OBJECT

 public:
  explicit KeymapConfigView(QWidget* parent = 0);
  ~KeymapConfigView();

 private:
  Ui::KeymapConfigView* ui;
};
