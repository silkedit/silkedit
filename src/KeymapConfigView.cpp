#include <QTableWidgetItem>

#include "KeymapConfigView.h"
#include "ui_KeymapConfigView.h"
#include "KeymapManager.h"

KeymapConfigView::KeymapConfigView(QWidget* parent)
    : QWidget(parent), ui(new Ui::KeymapConfigView) {
  ui->setupUi(this);
  ui->keymapTable->setRowCount(KeymapManager::singleton().keymaps().size());
  QStringList headerLabels{tr("Command"), tr("Key"), tr("If"), tr("Source")};
  ui->keymapTable->setColumnCount(headerLabels.size());
  ui->keymapTable->setHorizontalHeaderLabels(headerLabels);

  int row = 0;
  for (const auto& it : KeymapManager::singleton().keymaps()) {
    QString cmdName = it.second.cmdName();
    QTableWidgetItem* cmdNameItem = new QTableWidgetItem(cmdName);
    ui->keymapTable->setItem(row, 0, cmdNameItem);

    QString keySeqText = it.first.toString(QKeySequence::NativeText);
    QTableWidgetItem* keySeqItem = new QTableWidgetItem(keySeqText);
    ui->keymapTable->setItem(row, 1, keySeqItem);

    if (auto cond = it.second.condition()) {
      QTableWidgetItem* condItem = new QTableWidgetItem(cond->toString());
      ui->keymapTable->setItem(row, 2, condItem);
    }

    if (!it.second.source().isEmpty()) {
      QTableWidgetItem* sourceItem = new QTableWidgetItem(it.second.source());
      ui->keymapTable->setItem(row, 3, sourceItem);
    }
    row++;
  }
  ui->keymapTable->resizeColumnsToContents();

  setLayout(ui->rootLayout);
}

KeymapConfigView::~KeymapConfigView() {
  delete ui;
}
