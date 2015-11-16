#pragma once

#include <string>
#include <QDialog>
#include <QVariant>

#include "core/ConfigDefinition.h"

namespace Ui {
class ConfigDialog;
}

class PackagesView;

class ConfigDialog : public QDialog {
  Q_OBJECT

 public:
  static void loadConfig(const QString& pkgName, const QString& configPath);
  static void showModeless();

 private:
  static QMap<QString, QList<core::ConfigDefinition>> s_packageConfigs;
  static ConfigDialog* s_dialog;

  Ui::ConfigDialog* ui;
  PackagesView* m_packagesView;

  explicit ConfigDialog(QWidget* parent = 0);
  ~ConfigDialog();
  void filterConfigs(const QString& text);
};
