#include <QDebug>

#include "ToggleVimEmulationCommand.h"
#include "ViEngine.h"

ToggleVimEmulationCommand::ToggleVimEmulationCommand(ViEngine* viEngine)
    : ICommand(ToggleVimEmulationCommand::name), m_viEngine(viEngine) {}

const QString ToggleVimEmulationCommand::name = "toggle_vim_emulation";

void ToggleVimEmulationCommand::doRun(const CommandArgument&, int) {
  if (m_viEngine->isEnabled()) {
    qDebug() << "disable Vim emulation";
    m_viEngine->disable();
  } else {
    qDebug() << "enable Vim emulation";
    m_viEngine->enable();
  }
}
