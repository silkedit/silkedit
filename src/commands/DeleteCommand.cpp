#include <QDebug>
#include <QTextCursor>

#include "DeleteCommand.h"
#include "vi.h"
#include "stlSpecialization.h"

DeleteCommand::DeleteCommand(TextEditView* textEditView)
    : ICommand("delete"), m_textEditView(textEditView) {
}

void DeleteCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto direction = args.find<QString>("direction")) {
    if (*direction == "backward") {
      m_textEditView->doDelete(-repeat);
    } else if (*direction == "forward") {
      m_textEditView->doDelete(repeat);
    } else {
      qWarning() << "invalid direction";
    }
  } else {
    m_textEditView->doDelete(repeat);
  }
}
