#include <QDebug>

#include "UndoCommand.h"

UndoCommand::UndoCommand(ViEditView* viEditView) : ICommand("undo"), m_viEditView(viEditView) {
}

void UndoCommand::doRun(const CommandArgument& args, int repeat) {
  m_viEditView->doUndo(repeat);
}
