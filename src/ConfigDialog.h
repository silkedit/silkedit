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
  // Note: Ideally, this method should belong to core::Config. But this relies on YAMLUtil
  // which is not in core, so core::Config can't use it.
  static void loadDefinition(const QString& pkgName, const QString& configPath);
  static void showModeless();

 private:
  static QMap<QString, QList<core::ConfigDefinition>> s_packageConfigs;
  static ConfigDialog* s_dialog;

  Ui::ConfigDialog* ui;
  PackagesView* m_packagesView;

  explicit ConfigDialog(QWidget* parent = 0);
  ~ConfigDialog();
  void filterConfigs(const QString& text);
  void removePackageConfig(const QString& pkgName);
  void addPackageConfig(const QString& pkgName, QList<core::ConfigDefinition> configList);
};
