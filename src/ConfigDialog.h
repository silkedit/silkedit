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
  static void loadConfig(const QString& pkgName, const std::string& configPath);

  explicit ConfigDialog(QWidget* parent = 0);
  ~ConfigDialog();

 private:
  Ui::ConfigDialog* ui;
  PackagesView* m_packagesView;
  static QMap<QString, QList<core::ConfigDefinition>> s_packageConfigs;
};
