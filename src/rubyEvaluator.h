#pragma once

#include <QString>
#include "singleton.h"

class RubyEvaluator : public Singleton<RubyEvaluator> {
public:
  void eval(const QString &text);
  ~RubyEvaluator();

private:
  friend class Singleton<RubyEvaluator>;
  RubyEvaluator();
};
