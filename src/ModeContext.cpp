#include <QDebug>
#include "ModeContext.h"
#include "vi.h"

const QString ModeContext::name = "mode";

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

ModeContextCreator::ModeContextCreator(ViEngine* viEngine) : m_viEngine(viEngine) {
}

std::shared_ptr<IContext> ModeContextCreator::create(Operator op, const QString& operand) {
  return std::shared_ptr<IContext>(new ModeContext(m_viEngine, op, operand));
}
