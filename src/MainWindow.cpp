#include "MainWindow.h"
#include "KeymapService.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  m_editor.reset(new ViEditView);
  setCentralWidget(m_editor.get());

  m_viEngine.reset(new ViEngine(m_editor.get(), this));

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(m_viEngine.get()));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  KeymapService::singleton().load();
}
