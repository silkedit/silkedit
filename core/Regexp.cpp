#include <oniguruma.h>
#include <memory>
#include <QString>
#include <QStringBuilder>
#include <QDebug>

#include "Regexp.h"
#include "scoped_guard.h"

namespace {
bool isMetaChar(const QChar& ch) {
  return ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == '|' ||
         ch == '-' || ch == '*' || ch == '.' || ch == '\\' || ch == '?' || ch == '+' || ch == '^' ||
         ch == '$' || ch == '#' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ||
         ch == '\f' || ch == '\v';
}
}

namespace core {

Regexp::~Regexp() {
  onig_free(m_reg);
  //  onig_end();
}

QString Regexp::escape(const QString& expr) {
  QString escapedStr;
  for (int i = 0; i < expr.length();) {
    // escape a meta char
    if (isMetaChar(expr[i])) {
      escapedStr = escapedStr % '\\';
    }

    escapedStr = escapedStr % expr[i];
    i++;
  }

  return escapedStr;
}

std::unique_ptr<Regexp> Regexp::compile(const QString& expr) {
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
  int r = onig_new(&reg, pattern, pattern + strlen((char*)pattern), ONIG_OPTION_CAPTURE_GROUP,
                   ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &einfo);
  if (r != ONIG_NORMAL) {
    unsigned char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r, &einfo);
    qWarning() << "Onigmo warning:" << QString::fromUtf8((const char*)s) << "expr:" << expr;
    return nullptr;
  }

  Q_ASSERT(reg);
  return std::move(std::unique_ptr<Regexp>(new Regexp(reg, expr)));
}

// https://golang.org/pkg/regexp/#Regexp.FindAllStringSubmatchIndex
QVector<QVector<int>> Regexp::findAllStringSubmatchIndex(const QString& text,
                                                         int begin,
                                                         int end,
                                                         bool findNotEmpty) const {
  Q_ASSERT(m_reg);

  unsigned char *start, *range, *endOfStr;

  // todo: do we need to convert to UTF-8? Why not just use UTF-16 encoding used in Qt internally?
  QByteArray ba = text.toUtf8();
  unsigned char* str = (unsigned char*)ba.constData();
  endOfStr = str + ba.size();
  start = str + text.leftRef(begin).toUtf8().size();
  range = str + text.leftRef(end).toUtf8().size();

  QVector<QVector<int>> allIndices;

  while (true) {
    OnigRegion* region = onig_region_new();
    scoped_guard guard(
        [=] { onig_region_free(region, 1 /* 1:free self, 0:free contents only */); });

    int r = onig_search(m_reg, str, endOfStr, start, range, region, ONIG_OPTION_NONE);

    if (findNotEmpty && region->beg[0] == region->end[0]) {
      r = ONIG_MISMATCH;
    }

    if (r >= 0) {
      QVector<int> indices(region->num_regs * 2);

      for (int i = 0; i < region->num_regs; i++) {
        // Convert from byte offset to char offset in utf-8 string
        int begCharPos = region->beg[i] < 0
                             ? region->beg[i]
                             : onigenc_strlen(ONIG_ENCODING_UTF8, str, (str + region->beg[i]));
        indices[i * 2] = begCharPos;
        int endCharPos = region->end[i] < 0
                             ? region->end[i]
                             : begCharPos + onigenc_strlen(ONIG_ENCODING_UTF8, str + region->beg[i],
                                                           str + region->end[i]);
        indices[i * 2 + 1] = endCharPos;
      }

      if (indices.size() <= 1)
        break;

      allIndices.append(indices);

      start = str + region->end[0];

      if (indices.at(0) == indices.at(1)) {
        start += onig_enc_len(ONIG_ENCODING_UTF8, start, nullptr);
      }
    } else if (r == ONIG_MISMATCH) {
      break;
    } else { /* error */
      unsigned char s[ONIG_MAX_ERROR_MESSAGE_LEN];
      onig_error_code_to_str(s, r);
      qWarning() << QString::fromUtf8((const char*)s);
      break;
    }
  }

  return allIndices;
}

QVector<int> Regexp::onigSearch(unsigned char* str,
                                unsigned char* endOfStr,
                                unsigned char* start,
                                unsigned char* range,
                                bool findNotEmpty) const {
  OnigRegion* region = onig_region_new();
  scoped_guard guard([=] { onig_region_free(region, 1 /* 1:free self, 0:free contents only */); });

  unsigned char* gpos = start ? start : str;
  int r = onig_search_gpos(m_reg, str, endOfStr, gpos, start, range, region, ONIG_OPTION_NONE);

  // ONIG_OPTION_FIND_NOT_EMPTY doesn't work...
  if (findNotEmpty && region->beg[0] == region->end[0]) {
    r = ONIG_MISMATCH;
  }

  if (r >= 0) {
    QVector<int> indices(region->num_regs * 2);

    for (int i = 0; i < region->num_regs; i++) {
      // Convert from byte offset to char offset in utf-8 string
      int begCharPos = region->beg[i] < 0 ? region->beg[i] : onigenc_strlen(ONIG_ENCODING_UTF8, str,
                                                                            (str + region->beg[i]));
      indices[i * 2] = begCharPos;
      int endCharPos = region->end[i] < 0
                           ? region->end[i]
                           : begCharPos + onigenc_strlen(ONIG_ENCODING_UTF8, str + region->beg[i],
                                                         str + region->end[i]);
      indices[i * 2 + 1] = endCharPos;
    }

    return indices;
  } else if (r == ONIG_MISMATCH) {
    //    qDebug("search fail");
  } else { /* error */
    unsigned char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r);
    qWarning() << QString::fromUtf8((const char*)s);
  }

  return QVector<int>();
}

QVector<int> Regexp::findStringSubmatchIndex(const QString& text,
                                             int begin,
                                             int end,
                                             bool backward,
                                             bool findNotEmpty) const {
  return findStringSubmatchIndex(QStringRef(&text), begin, end, backward, findNotEmpty);
}

// https://golang.org/pkg/regexp/#Regexp.FindStringSubmatchIndex
QVector<int> Regexp::findStringSubmatchIndex(const QStringRef& text,
                                             int begin,
                                             int end,
                                             bool backward,
                                             bool findNotEmpty) const {
  Q_ASSERT(m_reg);

  unsigned char *start, *range, *endOfStr;

  QByteArray ba = text.toUtf8();
  unsigned char* str = (unsigned char*)ba.constData();
  endOfStr = str + ba.size();

  int bytesOfStrForBegin = text.left(begin).toUtf8().size();
  int bytesOfStrForEnd = text.left(end).toUtf8().size();

  if (backward) {
    start = str + bytesOfStrForEnd;
    range = str + bytesOfStrForBegin;
  } else {
    start = str + bytesOfStrForBegin;
    range = str + bytesOfStrForEnd;
  }

  return onigSearch(str, endOfStr, start, range, findNotEmpty);
}

bool Regexp::matches(const QString& text, bool findNotEmpty) {
  return !findStringSubmatchIndex(text, 0, -1, false, findNotEmpty).isEmpty();
}

Regexp::Regexp(regex_t* reg, const QString& pattern) : m_reg(reg), m_pattern(pattern) {}

}  // namespace core
