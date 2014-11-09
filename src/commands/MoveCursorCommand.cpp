#include <QDebug>
#include <QTextCursor>

#include "MoveCursorCommand.h"
#include "vi.h"
#include "stlSpecialization.h"

namespace {

int toMoveOperation(const QString& str) {
  if (str.toLower() == "up") {
    return QTextCursor::Up;
  } else if (str.toLower() == "down") {
    return QTextCursor::Down;
  } else if (str.toLower() == "left") {
    return QTextCursor::Left;
  } else if (str.toLower() == "right") {
    return QTextCursor::Right;
  } else {
    return QTextCursor::NoMove;
  }
}
}

MoveCursorCommand::MoveCursorCommand(ViEditView* viEditView)
    : ICommand("move_cursor"), m_viEditView(viEditView) {
}

void MoveCursorCommand::doRun(const CommandArgument& args, int repeat) {
  if (auto operationStr = args.find<QString>("operation")) {
    int operation = toMoveOperation(*operationStr);
    m_viEditView->moveCursor(operation, repeat);
  }
}
