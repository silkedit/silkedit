#include "MainWindow.h"
#include "KeymapService.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  m_textEditView.reset(new TextEditView);
  setCentralWidget(m_textEditView.get());

  m_viEngine.reset(new ViEngine(m_textEditView.get(), this));

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(m_viEngine.get()));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  KeymapService::singleton().load();
}
