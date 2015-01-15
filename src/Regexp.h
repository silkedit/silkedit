#pragma once

#include <QVector>

#include "macros.h"

struct re_pattern_buffer;
typedef re_pattern_buffer OnigRegexType;
typedef OnigRegexType regex_t;

class Regexp {
  DISABLE_COPY(Regexp)

 public:
  /*
   * 6 syntax:     address of pattern syntax definition.

      ONIG_SYNTAX_ASIS              plain text
      ONIG_SYNTAX_POSIX_BASIC       POSIX Basic RE
      ONIG_SYNTAX_POSIX_EXTENDED    POSIX Extended RE
      ONIG_SYNTAX_EMACS             Emacs
      ONIG_SYNTAX_GREP              grep
      ONIG_SYNTAX_GNU_REGEX         GNU regex
      ONIG_SYNTAX_JAVA              Java (Sun java.util.regex)
      ONIG_SYNTAX_PERL              Perl
      ONIG_SYNTAX_PERL_NG           Perl + named group
      ONIG_SYNTAX_RUBY              Ruby
      ONIG_SYNTAX_DEFAULT           default (== Ruby)
                                   onig_set_default_syntax()

      or any OnigSyntaxType data address defined by user.

      http://www.geocities.jp/kosako3/oniguruma/doc/API.txt
      http://www.geocities.jp/kosako3/oniguruma/doc/API.ja.txt
  */
  enum PatternSyntax {
    ASIS,
    Default,
  };

  ~Regexp();
  DEFAULT_MOVE(Regexp)

  static Regexp* compile(const QString& expr, PatternSyntax syntax = Default);

  QString pattern() { return m_pattern; }
  QVector<int>* findStringSubmatchIndex(const QStringRef& s, bool backward = false) const;

 private:
  regex_t* m_reg;
  QString m_pattern;

  Regexp(regex_t* reg, const QString& pattern);
};
