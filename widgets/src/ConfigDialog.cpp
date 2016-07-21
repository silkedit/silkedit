#include <QListWidgetItem>
#include <QStackedWidget>
#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "PackagesView.h"
#include "PackageConfigView.h"
#include "util/YamlUtil.h"
#include "core/Config.h"
#include "core/PackageManager.h"

using core::ConfigDefinition;
using core::Config;
using core::PackageManager;
using core::Package;

QMap<QString, QList<ConfigDefinition>> ConfigDialog::s_packageConfigs;
ConfigDialog* ConfigDialog::s_dialog = nullptr;

void ConfigDialog::loadDefinition(const QString& pkgName, const QString& configPath) {
  //  qDebug() << "loadDefinition" << pkgName;
  auto configList = YamlUtil::parseConfig(pkgName, configPath);
  if (!configList.isEmpty()) {
    s_packageConfigs[pkgName] = configList;
  }

  for (const auto& def : configList) {
    Config::singleton().addPackageConfigDefinition(def);
  }

  if (s_dialog && s_dialog->isVisible()) {
    s_dialog->addPackageConfig(pkgName, configList);
  }
}

void ConfigDialog::showModeless() {
  if (!s_dialog) {
    s_dialog = new ConfigDialog();
  }
  s_dialog->show();
  s_dialog->activateWindow();
  s_dialog->raise();
}

ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigDialog) {
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  ui->stackedWidget->setContentsMargins(5, 5, 5, 5);
  ui->contentLayout->setStretchFactor(ui->stackedWidget, 1);
  ui->contentLayout->addStretch();

  ui->listWidget->addItems(s_packageConfigs.keys());
  for (const auto& pkgDef : s_packageConfigs.values()) {
    ui->stackedWidget->addWidget(new PackageConfigView(pkgDef));
  }

  ui->listWidget->setCurrentRow(0);

  connect(ui->listWidget, &QListWidget::currentItemChanged,
          [&](QListWidgetItem* current, QListWidgetItem* previous) {
            if (!current) {
              current = previous;
            }
            int index = ui->listWidget->row(current);
            Q_ASSERT(0 <= index && index < ui->stackedWidget->count());
            ui->stackedWidget->setCurrentIndex(index);
          });
  connect(ui->filterLine, &QLineEdit::textEdited, this, &ConfigDialog::filterConfigs);
  connect(ui->okButton, &QPushButton::clicked, this, &ConfigDialog::done);
  connect(this, &ConfigDialog::finished, this, [=] { s_dialog = nullptr; });
  connect(&PackageManager::singleton(), &PackageManager::packageRemoved, this,
          [=](const Package& pkg) { removePackageConfig(pkg.name); });
}

ConfigDialog::~ConfigDialog() {
  qDebug("~ConfigDialog");
  delete ui;
}

void ConfigDialog::filterConfigs(const QString& text) {
  bool hasMatch = false;
  Filtering* filteringView = nullptr;
  for (int i = 0; i < ui->stackedWidget->count(); i++) {
    filteringView = nullptr;
    if (auto view = qobject_cast<PackageConfigView*>(ui->stackedWidget->widget(i))) {
      filteringView = view;
    } else if (auto view = qobject_cast<GeneralConfigView*>(ui->stackedWidget->widget(i))) {
      filteringView = view;
    }

    hasMatch = false;
    // If label of ListWidget matches, show all its configs
    if (Filtering::contains(ui->listWidget->item(i)->text(), text)) {
      hasMatch = true;
      if (filteringView) {
        filteringView->resetFilter();
      }
      // If label of ListWidget doesn't match, filter its configs
    } else if (filteringView) {
      hasMatch = filteringView->filter(text);
    }

    // If neither label of ListWidget nor any its configs match, hide it completely.
    ui->listWidget->item(i)->setHidden(!hasMatch);
  }

  // Move focus to the first visible row
  int row = 0;
  while (row < ui->listWidget->count() && ui->listWidget->item(row)->isHidden())
    row++;
  if (row < ui->listWidget->count()) {
    ui->listWidget->setCurrentRow(row);
  }
}

void ConfigDialog::removePackageConfig(const QString& pkgName) {
  s_packageConfigs.remove(pkgName);
  auto items = ui->listWidget->findItems(pkgName, Qt::MatchExactly);
  for (auto item : items) {
    int row = ui->listWidget->row(item);
    if (row >= 0) {
      ui->stackedWidget->removeWidget(ui->stackedWidget->widget(row));
    }
    // Deleting QListWidgetItem deletes an entry from QListWidget
    delete item;
  }
}

void ConfigDialog::addPackageConfig(const QString& pkgName,
                                    QList<core::ConfigDefinition> configList) {
//  qDebug() << "addPackageConfig" << pkgName;
#ifdef QT_DEBUG
  auto items = ui->listWidget->findItems(pkgName, Qt::MatchExactly);
  Q_ASSERT(items.empty());
#endif
  ui->listWidget->addItem(pkgName);
  ui->stackedWidget->addWidget(new PackageConfigView(configList));
}
