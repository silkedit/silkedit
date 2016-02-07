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
  boost::optional<QVector<int> > findStringSubmatchIndex(const QStringRef& s,
                                        bool backward = false,
                                        bool findNotEmpty = false) const;
  bool matches(const QString& text, bool findNotEmpty = false);

 private:
  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
};

}  // namespace core
