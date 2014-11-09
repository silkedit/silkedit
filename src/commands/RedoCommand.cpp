#include <QDebug>

#include "RedoCommand.h"

RedoCommand::RedoCommand(ViEditView* viEditView) : ICommand("redo"), m_viEditView(viEditView) {
}

void RedoCommand::doRun(const CommandArgument& args, int repeat) {
  m_viEditView->doRedo(repeat);
}
