#pragma once

#include <QVector>

#include "macros.h"

struct re_pattern_buffer;
typedef re_pattern_buffer OnigRegexType;
typedef OnigRegexType regex_t;

class Regexp {
  DISABLE_COPY(Regexp)

 public:
  ~Regexp();
  DEFAULT_MOVE(Regexp)

  static Regexp* compile(const QString& expr);
  static QString escape(const QString& expr);

  QString pattern() const { return m_pattern; }
  QVector<int>* findStringSubmatchIndex(const QStringRef& s,
                                        bool backward = false,
                                        bool findNotEmpty = false) const;

 private:
  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
};
