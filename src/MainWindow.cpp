#include "MainWindow.h"
#include "KeymapService.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  m_editor.reset(new ViEditView);
  setCentralWidget(m_editor.get());

  m_viEngine.reset(new ViEngine(m_editor.get(), this));
  KeymapService::singleton().load("keymap.yml");
}
