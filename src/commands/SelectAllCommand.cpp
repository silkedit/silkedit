#include <QDebug>

#include "SelectAllCommand.h"
#include "API.h"
#include "TextEditView.h"

const QString SelectAllCommand::name = "select_all";

SelectAllCommand::SelectAllCommand() : ICommand(name) {
}

void SelectAllCommand::doRun(const CommandArgument&, int) {
  API::activeEditView()->selectAll();
}
