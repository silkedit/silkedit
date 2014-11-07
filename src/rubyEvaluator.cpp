#include <QDebug>
#include <ruby.h>
#include "RubyEvaluator.h"

RubyEvaluator::RubyEvaluator() {
  qDebug() << "constructor of RubyEvaluator called";
  ruby_init();
  ruby_init_loadpath();
}

RubyEvaluator::~RubyEvaluator() {
  qDebug() << "destructor of RubyEvaluator called";
  ruby_cleanup(0);
}

// FIXME: make this thread safe!
void RubyEvaluator::eval(const QString& text) {
  int state;
  qDebug() << "evaluated ruby code" << text;
  rb_eval_string_protect(text.toStdString().c_str(), &state);
  if (state) {
    VALUE err = rb_String(rb_errinfo());
    qWarning() << StringValueCStr(err);
  }
}
