#include <QFile>
#include <QDebug>

#include "AboutDialog.h"
#include "App.h"
#include "version.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  const QString text = tr("version %1 ( %2 )").arg(App::applicationVersion()).arg(COMMIT);
  ui->buildLabel->setText(text);
}

AboutDialog::~AboutDialog() {
  delete ui;
}
