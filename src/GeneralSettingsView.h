#pragma once

#include <QWidget>

namespace Ui {
class GeneralSettingsView;
}

class GeneralSettingsView : public QWidget {
  Q_OBJECT

 public:
  explicit GeneralSettingsView(QWidget* parent = 0);
  ~GeneralSettingsView();

 private:
  Ui::GeneralSettingsView* ui;
};
