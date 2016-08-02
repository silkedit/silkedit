#include <QFile>
#include <QDebug>

#include "AboutDialog.h"
#include "App.h"
#include "version.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  ui->buildLabel->setText(tr("version") + " " + App::applicationVersion() + " (" + tr("build") +
                          ": " + BUILD + ")");
}

AboutDialog::~AboutDialog() {
  delete ui;
}
