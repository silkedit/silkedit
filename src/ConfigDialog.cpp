#include <QListWidgetItem>
#include <QStackedWidget>
#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "PackagesView.h"

ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigDialog) {
  ui->setupUi(this);
  int sizeHint = ui->listWidget->sizeHintForColumn(0);
  ui->listWidget->setFixedWidth(sizeHint);
  ui->lineEdit->setFixedWidth(ui->listWidget->width());
  ui->stackedWidget->setContentsMargins(5, 5, 5, 5);

  connect(ui->listWidget, &QListWidget::currentItemChanged,
          [&](QListWidgetItem* current, QListWidgetItem* previous) {
            if (!current)
              current = previous;
            ui->stackedWidget->setCurrentIndex(ui->listWidget->row(current));
          });

  ui->listWidget->setCurrentRow(0);
  setLayout(ui->rootHLayout);
}

ConfigDialog::~ConfigDialog() {
  delete ui;
}
