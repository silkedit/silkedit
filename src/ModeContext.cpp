#include <QDebug>
#include "ModeContext.h"
#include "vi.h"

const QString ModeContext::name = "mode";

bool ModeContext::isSatisfied(Operator op, const QString &operand)
{
  return m_viEngine->isEnabled() && IContext::isSatisfied(op, operand);
}

QString ModeContext::key() {
  switch (m_viEngine->mode()) {
    case Mode::CMD:
      return "normal";
    case Mode::INSERT:
      return "insert";
    case Mode::CMDLINE:
      return "commnad_line";
    default:
      qDebug() << "invalid mode";
      return "";
  }
}
