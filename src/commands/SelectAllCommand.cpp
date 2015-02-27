#include <QDebug>

#include "SelectAllCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

const QString SelectAllCommand::name = "select_all";

SelectAllCommand::SelectAllCommand() : ICommand(name) {
}

void SelectAllCommand::doRun(const CommandArgument&, int) {
  SilkApp::activeEditView()->selectAll();
}
