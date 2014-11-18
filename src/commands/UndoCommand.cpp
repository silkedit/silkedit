#include <QDebug>

#include "UndoCommand.h"

UndoCommand::UndoCommand(TextEditView* textEditView)
    : ICommand("undo"), m_textEditView(textEditView) {
}

void UndoCommand::doRun(const CommandArgument&, int repeat) {
  m_textEditView->doUndo(repeat);
}
