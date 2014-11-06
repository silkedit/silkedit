#include <QDebug>
#include "ModeContext.h"
#include "vi.h"

ModeContext::ModeContext(ViEngine* viEngine, Operator op, const QString& operand)
    : IContextBase(op, operand), m_viEngine(viEngine) {
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
