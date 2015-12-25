#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#pragma warning(disable : 4091)
#endif

#include "HttpSendDump.h"
#include <qdebug.h>
#include <qstring.h>
#include <qmessagebox.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  Qt::WindowFlags flags = Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint;
  flags &= ~Qt::WindowMaximizeButtonHint;
  setWindowFlags(flags);

  ui->setupUi(this);

  if (qApp->arguments().count() >= 2) {
    fileName = qApp->arguments().at(1);
  }
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_pushButton_send_clicked() {
  if (fileName.isEmpty()) {
    qDebug() << "FileName empty";
    return;
  }

  comment = ui->plainTextEdit->toPlainText();

  CrashReport::HttpSendDump sender(fileName, comment, CRASH_APP_VERSION);
  QString ret = sender.sendDump();

  int i = ret.indexOf("\"success\"");
  QString msg;
  if(i>=0) {
    msg = tr("Sent a bug report successfully.");
  } else {
    msg = tr("Failed to send a bug report.");
  }

  QMessageBox msgBox(this);
  msgBox.setText(msg);
  msgBox.exec();

  qApp->quit();
}

void MainWindow::on_pushButton_exit_clicked() {
  qApp->quit();
}
