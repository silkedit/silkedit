#include <QDebug>

#include "RedoCommand.h"

RedoCommand::RedoCommand(TextEditView* textEditView)
    : ICommand("redo"), m_textEditView(textEditView) {
}

void RedoCommand::doRun(const CommandArgument&, int repeat) {
  m_textEditView->doRedo(repeat);
}
