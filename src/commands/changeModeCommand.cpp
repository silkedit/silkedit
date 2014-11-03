#include <QDebug>

#include "changeModeCommand.h"
#include "../vi.h"

ChangeModeCommand::ChangeModeCommand(ViEngine *viEngine): ICommand("change_mode"), m_viEngine(viEngine) {}

void ChangeModeCommand::doRun()
{
  m_viEngine->setMode(INSERT);
}
