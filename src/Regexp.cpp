#include <oniguruma.h>
#include <QString>
#include <QDebug>

#include "Regexp.h"

Regexp::~Regexp() {
  onig_free(m_reg);
  //  onig_end();
}

Regexp* Regexp::compile(const QString& expr, PatternSyntax syntax) {
  //  qDebug("compile Regexp: %s", qPrintable(expr));
  regex_t* reg = nullptr;
  OnigErrorInfo einfo;

  QByteArray ba = expr.toUtf8();
  unsigned char* pattern = (unsigned char*)ba.constData();
  Q_ASSERT(pattern);

  OnigSyntaxType* onigSyntax;
  switch (syntax) {
  case ASIS:
    onigSyntax = ONIG_SYNTAX_ASIS;
    break;
  default:
    onigSyntax = ONIG_SYNTAX_DEFAULT;
    break;
  }

  // todo: check encoding!
  int r = onig_new(&reg,
                   pattern,
                   pattern + strlen((char*)pattern),
                   ONIG_OPTION_CAPTURE_GROUP,
                   ONIG_ENCODING_UTF8,
                   onigSyntax,
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

Regexp::Regexp(regex_t* reg, const QString& pattern) : m_reg(reg), m_pattern(pattern) {
}
