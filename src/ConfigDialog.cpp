#include <QListWidgetItem>
#include <QStackedWidget>
#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "PackagesView.h"
#include "PackageConfigView.h"
#include "util/YamlUtils.h"

using core::ConfigDefinition;

QMap<QString, QList<ConfigDefinition>> ConfigDialog::s_packageConfigs;
ConfigDialog* ConfigDialog::s_dialog = nullptr;

void ConfigDialog::loadConfig(const QString& pkgName, const QString& configPath) {
  auto configList = YamlUtils::parseConfig(pkgName, configPath);
  if (!configList.isEmpty()) {
    s_packageConfigs[pkgName] = configList;
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
  ui->stackedWidget->setContentsMargins(5, 5, 5, 5);
  ui->contentLayout->setStretchFactor(ui->stackedWidget, 1);
  ui->contentLayout->addStretch();
  setLayout(ui->rootLayout);

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
            ui->stackedWidget->setCurrentIndex(index);
          });

  connect(ui->filterLine, &QLineEdit::textEdited, this, &ConfigDialog::filterConfigs);
  connect(ui->okButton, &QPushButton::clicked, this, &ConfigDialog::done);
  connect(this, &ConfigDialog::finished, this, [=] { s_dialog = nullptr; });
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
