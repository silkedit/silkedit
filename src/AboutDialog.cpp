#include <QFile>
#include <QDebug>

#include "AboutDialog.h"
#include "App.h"
#include "version.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  QFile license(":/LICENSE.md");
  if (!license.open(QIODevice::ReadOnly)) {
    qCritical() << "failed to open LICENSE in resource";
    return;
  }

  ui->licenseTextEdit->setPlainText(license.readAll());
  ui->buildLabel->setText(tr("version") + " " + App::applicationVersion() + " (" + tr("build") +
                         ": " + BUILD + ")");
}

AboutDialog::~AboutDialog() {
  delete ui;
}
