#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qdebug.h>
#include <qstring.h>

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
      if (first) {
        qDebug() << "first chance";
        QString fileName = "";
        if(qApp->arguments().count() >= 2) {
          fileName = qApp->arguments().at(1);
        }
        showDump(fileName);
      }
      // turn off
      first = false;
      break;
    default:
      break;
  }
}

void MainWindow::showDump(const QString& fileName) {

  QString msg = tr("Silkedit crashed.") + "\n" + fileName;

  QFont f;
  f.setPointSize(12);
  ui->label->setFont(f);

  //ui->label->setText(msg);
  QMetaObject::invokeMethod(ui->label, "setText", Qt::QueuedConnection, Q_ARG(QString, msg));
}
