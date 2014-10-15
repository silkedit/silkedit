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
  statusBar()->addWidget(m_cmdLineEdit = new QLineEdit(), 1);

  connect(m_viEngine, SIGNAL(modeChanged(Mode)), this,
          SLOT(onModeChanged(Mode)));
  connect(m_viEngine, SIGNAL(modeChanged(Mode)), m_editor, SLOT(setMode(Mode)));
  connect(m_cmdLineEdit, SIGNAL(returnPressed()), this,
          SLOT(cmdLineReturnPressed()));

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
  case CMDLINE:
    m_cmdLineEdit->setText(":");
    m_cmdLineEdit->show();
    m_cmdLineEdit->setFocus(Qt::OtherFocusReason);
    return;
  }

  m_cmdLineEdit->hide();
  statusBar()->showMessage(text);
}

void MainWindow::cmdLineReturnPressed() {
  QString text = m_cmdLineEdit->text();
  if (!text.isEmpty() && text[0] == ':') {
    m_viEngine->processExCommand(text.mid(1));
  }
}
