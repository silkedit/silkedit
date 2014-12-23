#include <QDebug>

#include "DeleteCommand.h"
#include "API.h"
#include "TextEditView.h"

DeleteCommand::DeleteCommand() : ICommand("delete") {}

void DeleteCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto direction = args.find<QString>("direction")) {
    if (*direction == "backward") {
      API::activeEditView()->doDelete(-repeat);
    } else if (*direction == "forward") {
      API::activeEditView()->doDelete(repeat);
    } else {
      qWarning() << "invalid direction";
    }
  } else {
    API::activeEditView()->doDelete(repeat);
  }
}
