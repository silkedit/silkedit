#include <QDebug>

#include "DeleteCommand.h"
#include "SilkApp.h"
#include "TextEditView.h"

DeleteCommand::DeleteCommand() : ICommand("delete") {}

void DeleteCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto direction = args.find<QString>("direction")) {
    if (*direction == "backward") {
      SilkApp::activeEditView()->doDelete(-repeat);
    } else if (*direction == "forward") {
      SilkApp::activeEditView()->doDelete(repeat);
    } else {
      qWarning() << "invalid direction";
    }
  } else {
    SilkApp::activeEditView()->doDelete(repeat);
  }
}
