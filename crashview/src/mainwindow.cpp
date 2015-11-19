#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "crashreport.h"

#include <qdebug.h>
#include <qstring.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qfileinfo.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::changeEvent(QEvent* e) {
  qDebug() << QString("changeEvent:") << e->type();
  QMainWindow::changeEvent(e);

  // first chance flag
  static bool first = true;
  switch (e->type()) {
    case QEvent::WindowTitleChange:
      if (first && qApp->arguments().count() >= 2) {
        qDebug() << "first chance";
        showDump(qApp->arguments().at(1));
      }
      // turn off
      first = false;
      break;
    default:
      break;
  }
}

void MainWindow::on_actionExit_triggered() {
  qApp->quit();
}

void MainWindow::on_actionOpen_triggered() {
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::tempPath(),
                                                  tr("dump Files (*.dmp);;Text Files (*.txt)"));

  if (!fileName.isEmpty()) {
    QFileInfo info(fileName);
    if (info.suffix() == "dmp") {
      showDump(fileName);
    } else if (info.suffix() == "txt") {
      ui->textBrowser->setText(CrashReport::loadFile(fileName));
    }
  }
}

void MainWindow::showDump(const QString& fileName) {
  CrashReport crash;

  crash.setPath(fileName);
  crash.setToolPath(qApp->applicationDirPath());

  QString msg = crash.report();
  ui->textBrowser->setText(msg);
}
