#pragma once

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
  QVector<int> findStringSubmatchIndex(const QStringRef& text,
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
  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
  QVector<int> onigSearch(unsigned char* str,
                          unsigned char* endOfStr,
                          unsigned char* start,
                          unsigned char* range,
                          bool findNotEmpty) const;
};

}  // namespace core
