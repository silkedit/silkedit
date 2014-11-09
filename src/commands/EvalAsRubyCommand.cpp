#include <QDebug>

#include "EvalAsRubyCommand.h"
#include "RubyEvaluator.h"

EvalAsRubyCommand::EvalAsRubyCommand(ViEditView* viEditView)
    : ICommand("eval_as_ruby"), m_viEditView(viEditView) {
}

void EvalAsRubyCommand::doRun(const CommandArgument& args, int repeat) {
  RubyEvaluator::singleton().eval(m_viEditView->toPlainText());
}
