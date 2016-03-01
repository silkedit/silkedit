#pragma once

#include <oniguruma.h>
#include <memory>
#include <boost/optional.hpp>
#include <QVector>
#include <QStringRef>

#include "macros.h"

struct re_pattern_buffer;
typedef re_pattern_buffer OnigRegexType;
typedef OnigRegexType regex_t;

namespace core {

class Regexp {
  DISABLE_COPY(Regexp)

 public:
  ~Regexp();
  DEFAULT_MOVE(Regexp)

  static std::unique_ptr<Regexp> compile(const QString& expr);
  static QString escape(const QString& expr);

  QString pattern() const { return m_pattern; }

  QVector<int> findStringSubmatchIndex(const QString& text,
                                       int begin = 0,
                                       int end = -1,
                                       bool backward = false,
                                       bool findNotEmpty = false) const;
  bool matches(const QString& text, bool findNotEmpty = false);

  QVector<QVector<int>> findAllStringSubmatchIndex(const QString& text,
                                                   int begin = 0,
                                                   int end = -1,
                                                   bool findNotEmpty = false) const;

 private:
  static QMutex s_mutex;

  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
  QVector<int> onigSearch(const OnigUChar *str,
                          const OnigUChar *endOfStr,
                          const OnigUChar *start,
                          const OnigUChar *range,
                          bool findNotEmpty) const;
};

}  // namespace core
