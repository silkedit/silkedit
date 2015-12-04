#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qdebug.h>
#include <qstring.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QString fileName = "";
  if(qApp->arguments().count() >= 2) {
    fileName = qApp->arguments().at(1);
  }
  showDump(fileName);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::showDump(const QString& fileName) {

  QString msg = tr("Silkedit crashed.") + "\n" + fileName;

  QFont f;
  f.setPointSize(12);
  ui->label->setFont(f);

  ui->label->setText(msg);
}
