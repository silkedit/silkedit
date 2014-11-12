#include <QDebug>

#include "EvalAsRubyCommand.h"
#include "RubyEvaluator.h"

EvalAsRubyCommand::EvalAsRubyCommand(TextEditView* textEditView)
    : ICommand("eval_as_ruby"), m_textEditView(textEditView) {
}

void EvalAsRubyCommand::doRun(const CommandArgument&, int) {
  RubyEvaluator::singleton().eval(m_textEditView->toPlainText());
}
