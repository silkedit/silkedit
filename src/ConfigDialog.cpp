#include <QListWidgetItem>
#include <QStackedWidget>
#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "PackagesView.h"
#include "PackageConfigView.h"
#include "util/YamlUtils.h"

using core::ConfigDefinition;

QMap<QString, QList<ConfigDefinition>> ConfigDialog::s_packageConfigs;

void ConfigDialog::loadConfig(const QString& pkgName, const std::string& configPath) {
  s_packageConfigs[pkgName] = YamlUtils::parseConfig(pkgName, configPath);
  ;
}

ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigDialog) {
  ui->setupUi(this);
  int sizeHint = ui->listWidget->sizeHintForColumn(0);
  ui->listWidget->setFixedWidth(sizeHint);
  ui->lineEdit->setFixedWidth(ui->listWidget->width());
  ui->stackedWidget->setContentsMargins(5, 5, 5, 5);

  ui->listWidget->addItems(s_packageConfigs.keys());

  ui->listWidget->setCurrentRow(0);
  connect(ui->listWidget, &QListWidget::currentItemChanged,
          [&](QListWidgetItem* current, QListWidgetItem* previous) {
            if (!current) {
              current = previous;
            }
            int index = ui->listWidget->row(current);
            const QString& name = ui->listWidget->currentItem()->text();
            // create PackageConfigView for the first time
            if (!ui->stackedWidget->widget(index) && s_packageConfigs.contains(name)) {
              ui->stackedWidget->insertWidget(index, new PackageConfigView(s_packageConfigs[name]));
            }
            ui->stackedWidget->setCurrentIndex(index);
          });

  setLayout(ui->rootHLayout);
}

ConfigDialog::~ConfigDialog() {
  delete ui;
}
