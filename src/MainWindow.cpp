#include <QFileDialog>
#include <QMenuBar>
#include <QAction>

#include "DocumentService.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandAction.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/OpenFileCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  ConfigService::singleton().load();

  m_layoutView.reset(new LayoutView);
  DocumentService::singleton().setLayoutView(m_layoutView.get());

  setCentralWidget(m_layoutView.get());

  m_viEngine.reset(new ViEngine(m_layoutView.get(), this));

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    m_viEngine->enable();
  }

  std::unique_ptr<ToggleVimEmulationCommand> toggleVimEmulationCmd(
      new ToggleVimEmulationCommand(m_viEngine.get()));
  CommandService::singleton().add(std::move(toggleVimEmulationCmd));

  std::unique_ptr<OpenFileCommand> openFileCmd(new OpenFileCommand());
  CommandService::singleton().add(std::move(openFileCmd));

  // Load keymap settings after registering commands
  KeymapService::singleton().load();

  auto openFileAction = new CommandAction(tr("&Open..."), OpenFileCommand::name, this);

  auto fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openFileAction);
}
