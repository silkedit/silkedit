#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "HttpSendDump.h"
#include <qdebug.h>
#include <qstring.h>
#include <qmessagebox.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  Qt::WindowFlags flags = Qt::Window
                        | Qt::MSWindowsFixedSizeDialogHint;
                  flags &= ~Qt::WindowMaximizeButtonHint;
  setWindowFlags(flags);

  ui->setupUi(this);

  if(qApp->arguments().count() >= 2) {
    fileName = qApp->arguments().at(1);
  }
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_pushButton_send_clicked() {
  if(fileName.isEmpty()) {
    qDebug() << "FileName empty";
    return;
  }

  comment = ui->plainTextEdit->toPlainText();

  CrashReport::HttpSendDump sender(fileName,comment,CRASH_APP_VERSION);
  QString ret = sender.sendDump();

  QMessageBox msgBox(this);
  msgBox.setText(ret);
  msgBox.exec();
}

void MainWindow::on_pushButton_exit_clicked() {
  qApp->quit();
}
