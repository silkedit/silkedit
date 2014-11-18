#include <QFileDialog>

#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/OpenFileCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  ConfigService::singleton().load();

  m_textEditView.reset(new TextEditView);
  setCentralWidget(m_textEditView.get());

  m_viEngine.reset(new ViEngine(m_textEditView.get(), this));

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    m_viEngine->enable();
  }

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(m_viEngine.get()));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  std::unique_ptr<OpenFileCommand> openFileCmd(new OpenFileCommand(m_textEditView.get()));
  CommandService::singleton().add(std::move(openFileCmd));

  KeymapService::singleton().load();
}
