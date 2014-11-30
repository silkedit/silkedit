#include <QFileDialog>
#include <QMenuBar>
#include <QAction>

#include "API.h"
#include "DocumentService.h"
#include "MainWindow.h"
#include "KeymapService.h"
#include "ConfigService.h"
#include "CommandAction.h"
#include "CommandService.h"
#include "commands/ToggleVimEmulationCommand.h"
#include "commands/OpenFileCommand.h"
#include "commands/MoveCursorCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/UndoCommand.h"
#include "commands/RedoCommand.h"
#include "commands/EvalAsRubyCommand.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("SilkEdit"));

  ConfigService::singleton().load();

  m_layoutView.reset(new LayoutView);
  DocumentService::singleton().setTabWidget(m_layoutView->tabWidget());
  API::singleton().init(m_layoutView->tabWidget());

  setCentralWidget(m_layoutView.get());

  // Set focus to active edit view
  API::singleton().activeEditView()->setFocus();

  m_viEngine.reset(new ViEngine(this));

  if (ConfigService::singleton().isTrue("enable_vim_emulation")) {
    m_viEngine->enable();
  }

  // add commands
  std::unique_ptr<MoveCursorCommand> moveCursorCmd(new MoveCursorCommand);
  CommandService::singleton().add(std::move(moveCursorCmd));

  std::unique_ptr<DeleteCommand> deleteCmd(new DeleteCommand);
  CommandService::singleton().add(std::move(deleteCmd));

  std::unique_ptr<UndoCommand> undoCmd(new UndoCommand);
  CommandService::singleton().add(std::move(undoCmd));

  std::unique_ptr<RedoCommand> redoCmd(new RedoCommand);
  CommandService::singleton().add(std::move(redoCmd));

  std::unique_ptr<EvalAsRubyCommand> evalAsRubyCmd(new EvalAsRubyCommand);
  CommandService::singleton().add(std::move(evalAsRubyCmd));

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
