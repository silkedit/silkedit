#include <QDebug>

#include "changeToInsertModeCommand.h"
#include "../vi.h"

ChangeToInsertModeCommand::ChangeToInsertModeCommand(ViEngine *viEngine): ICommand("change_to_insert_mode"), m_viEngine(viEngine) {}

void ChangeToInsertModeCommand::doRun()
{
  m_viEngine->setMode(INSERT);
}
