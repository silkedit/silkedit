#include <QStatusBar>
#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
  this->setWindowTitle(QObject::tr("Code Editor Example"));
  m_editor = new ViEditView;
  setCentralWidget(m_editor);

  connect(m_editor, SIGNAL(modeChanged()), this, SLOT(onModeChanged()));
  onModeChanged();
}

MainWindow::~MainWindow() {}

void MainWindow::onModeChanged() {
  QString text;
  switch ( m_editor->mode()) {
  case ViEditView::CMD:
    text = "CMD";
    break;
  case ViEditView::INSERT:
    text = "INSERT";
    break;
  }

  statusBar()->showMessage(text);
}
