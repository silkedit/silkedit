#include <QStatusBar>

#include "vi.h"
#include "mainWindow.h"
#include "viEditView.h"
#include "viEngine.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("Code Editor Example"));

  m_viEngine = new ViEngine;
  m_editor = new ViEditView;
  setCentralWidget(m_editor);
  m_viEngine->setEditor(m_editor);

  connect(m_viEngine, SIGNAL(modeChanged(Mode)), this,
          SLOT(onModeChanged(Mode)));
  connect(m_viEngine, SIGNAL(modeChanged(Mode)), m_editor, SLOT(setMode(Mode)));
  onModeChanged(m_viEngine->mode());
}

MainWindow::~MainWindow() {}

void MainWindow::onModeChanged(Mode mode) {
  QString text;
  switch (mode) {
  case CMD:
    text = "CMD";
    break;
  case INSERT:
    text = "INSERT";
    break;
  }

  statusBar()->showMessage(text);
}
