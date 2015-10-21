#pragma once

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class PackagesView;

class ConfigDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ConfigDialog(QWidget* parent = 0);
  ~ConfigDialog();

 private:
  Ui::ConfigDialog* ui;
  PackagesView* m_packagesView;
};
