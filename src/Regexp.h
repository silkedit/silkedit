#pragma once

#include <oniguruma.h>
#include <QVector>

#include "macros.h"

class Regexp {
  DISABLE_COPY(Regexp)

 public:
  ~Regexp();
  DEFAULT_MOVE(Regexp)

  static Regexp* compile(const QString& expr);

  QString pattern() { return m_pattern; }
  QVector<int>* findStringSubmatchIndex(const QString& s);

 private:
  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
};
