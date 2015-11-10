#include <QTableWidgetItem>
#include <QFile>

#include "KeymapConfigView.h"
#include "ui_KeymapConfigView.h"
#include "KeymapManager.h"
#include "PluginManager.h"
#include "DocumentManager.h"
#include "Window.h"
#include "core/Constants.h"

using core::Constants;

KeymapConfigView::KeymapConfigView(QWidget* parent)
    : QWidget(parent), ui(new Ui::KeymapConfigView) {
  ui->setupUi(this);
  ui->keymapTable->setRowCount(KeymapManager::singleton().keymaps().size());
  QStringList headerLabels{tr("Command"), tr("Description"), tr("Key"), tr("If"), tr("Source")};
  ui->keymapTable->setColumnCount(headerLabels.size());
  ui->keymapTable->setHorizontalHeaderLabels(headerLabels);

  int row = 0;
  int column = 0;
  for (const auto& it : KeymapManager::singleton().keymaps()) {
    column = 0;
    QString cmdName = it.second.cmdName();
    QTableWidgetItem* cmdNameItem = new QTableWidgetItem(cmdName);
    ui->keymapTable->setItem(row, column, cmdNameItem);
    column++;

    QString description = it.second.cmdDescription();
    QTableWidgetItem* cmdDescriptionItem = new QTableWidgetItem(description);
    ui->keymapTable->setItem(row, column, cmdDescriptionItem);
    column++;

    QString keySeqText = it.first.toString(QKeySequence::NativeText);
    QTableWidgetItem* keySeqItem = new QTableWidgetItem(keySeqText);
    ui->keymapTable->setItem(row, column, keySeqItem);
    column++;

    if (auto cond = it.second.condition()) {
      QTableWidgetItem* condItem = new QTableWidgetItem(cond->toString());
      ui->keymapTable->setItem(row, column, condItem);
    }
    column++;

    if (!it.second.source().isEmpty()) {
      QTableWidgetItem* sourceItem = new QTableWidgetItem(it.second.source());
      ui->keymapTable->setItem(row, column, sourceItem);
    }
    row++;
  }
  ui->keymapTable->resizeColumnsToContents();

  connect(ui->openKeymapFileButton, &QPushButton::clicked, this, [=] {
    QFile keymapFile(Constants::userKeymapPath());
    if (!keymapFile.exists()) {
      if (keymapFile.open(QFile::WriteOnly | QIODevice::Text)) {
        QString defaultContent(R"(
# Define your custom keymap here
- keymap:
  # ctrl+b: move_cursor_left
)");

        QTextStream out(&keymapFile);
        out << defaultContent.trimmed();
      } else {
        qWarning("Failed to open %s", qPrintable(keymapFile.fileName()));
      }
    }
    Window::showFirst();
    DocumentManager::open(Constants::userKeymapPath());
  });

  setLayout(ui->rootLayout);
}

KeymapConfigView::~KeymapConfigView() {
  delete ui;
}
