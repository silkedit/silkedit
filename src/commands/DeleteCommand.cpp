#include <QDebug>
#include <QTextCursor>

#include "DeleteCommand.h"
#include "vi.h"
#include "stlSpecialization.h"

DeleteCommand::DeleteCommand(ViEditView* viEditView)
    : ICommand("delete"), m_viEditView(viEditView) {
}

void DeleteCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto direction = args.find<QString>("direction")) {
    if (*direction == "backward") {
      m_viEditView->doDelete(-repeat);
    } else if (*direction == "forward") {
      m_viEditView->doDelete(repeat);
    } else {
      qWarning() << "invalid direction";
    }
  } else {
    m_viEditView->doDelete(repeat);
  }
}
