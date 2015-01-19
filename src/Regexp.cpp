#include <oniguruma.h>
#include <QString>
#include <QStringBuilder>
#include <QDebug>

#include "Regexp.h"

namespace {
bool isMetaChar(const QChar& ch) {
  return ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == '|' ||
         ch == '-' || ch == '*' || ch == '.' || ch == '\\' || ch == '?' || ch == '+' || ch == '^' ||
         ch == '$' || ch == '#' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ||
         ch == '\f' || ch == '\v';
}
}

Regexp::~Regexp() {
  onig_free(m_reg);
  //  onig_end();
}

QString Regexp::escape(const QString& expr) {
  QString escapedStr;
  for (int i = 0; i < expr.length();) {
    // skip consecutive backslashes (\\)
    if (expr[i] == '\\' && i + 1 < expr.length() && expr[i + 1] == '\\') {
      escapedStr = escapedStr % "\\\\";
      i += 2;
      continue;
    }

    // single backslash. Check if it escapes a following meta char.
    if (expr[i] == '\\' && i + 1 < expr.length() && isMetaChar(expr[i + 1])) {
      escapedStr = escapedStr % "\\" % expr[i + 1];
      i += 2;
      continue;
    }

    // escape a meta char
    if (isMetaChar(expr[i])) {
      escapedStr = escapedStr % '\\';
    }

    escapedStr = escapedStr % expr[i];
    i++;
  }

  return escapedStr;
}

Regexp* Regexp::compile(const QString& expr) {
  //  qDebug("compile Regexp: %s", qPrintable(expr));
  if (expr.isEmpty()) {
    return nullptr;
  }

  regex_t* reg = nullptr;
  OnigErrorInfo einfo;

  QByteArray ba = expr.toUtf8();
  unsigned char* pattern = (unsigned char*)ba.constData();
  Q_ASSERT(pattern);

  // todo: check encoding!
  int r = onig_new(&reg,
                   pattern,
                   pattern + strlen((char*)pattern),
                   ONIG_OPTION_CAPTURE_GROUP,
                   ONIG_ENCODING_UTF8,
                   ONIG_SYNTAX_DEFAULT,
                   &einfo);
  if (r != ONIG_NORMAL) {
    unsigned char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r, &einfo);
    qWarning() << QString::fromUtf8((const char*)s);
    return nullptr;
  }

  Q_ASSERT(reg);
  return new Regexp(reg, expr);
}

QVector<int>* Regexp::findStringSubmatchIndex(const QStringRef& s, bool backward) const {
  Q_ASSERT(m_reg);

  unsigned char* start, *range, *end;
  OnigRegion* region = onig_region_new();

  QByteArray ba = s.toUtf8();
  unsigned char* str = (unsigned char*)ba.constData();
  end = str + strlen((char*)str);
  if (backward) {
    start = end;
    range = str;
  } else {
    start = str;
    range = end;
  }
  int r = onig_search(m_reg, str, end, start, range, region, ONIG_OPTION_NONE);

  QVector<int>* indices = nullptr;
  if (r >= 0) {
    int i;
    indices = new QVector<int>(0);

    for (i = 0; i < region->num_regs; i++) {
      indices->append(region->beg[i]);
      indices->append(region->end[i]);
    }
  } else if (r == ONIG_MISMATCH) {
    //    qDebug("search fail");
  } else { /* error */
    unsigned char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r);
    qWarning() << QString::fromUtf8((const char*)s);
  }

  onig_region_free(region, 1 /* 1:free self, 0:free contents only */);
  return indices;
}

Regexp::Regexp(regex_t* reg, const QString& pattern) : m_reg(reg), m_pattern(pattern) {}
