#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

#include "Util.h"

namespace {

void replace(QString& str, const QString& regex, const QString& after) {
  QRegularExpression re(regex, QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator iter = re.globalMatch(str);
  while (iter.hasNext()) {
    QRegularExpressionMatch match = iter.next();
    str = str.replace(match.capturedStart(), match.capturedLength(), after);
  }
}
}

namespace core {

// This function is same as sort.Search in golang
// Search uses binary search to find and return the smallest index i in [0, n) at which f(i) is
// true, assuming that on the range [0, n), f(i) == true implies f(i+1) == true. That is, Search
// requires that f is false for some (possibly empty) prefix of the input range [0, n) and then true
// for the (possibly empty) remainder; Search returns the first true index. If there is no such
// index, Search returns n. (Note that the "not found" return value is not -1 as in, for instance,
// strings.Index). Search calls f(i) only for i in the range [0, n).
size_t Util::binarySearch(size_t last, std::function<bool(size_t)> fn) {
  size_t low = 0;
  size_t high = last;
  while (low < high) {
    size_t mid = (low + high) / 2;
    //    qDebug("low: %d, high: %d, mid: %d", low, high, mid);
    if (fn(mid)) {
      if (mid == 0) {
        break;
      }
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }
  return low;
}

void Util::ensureDir(const QString& path) {
  QDir::root().mkpath(QFileInfo(path).dir().path());
}

bool Util::copy(const QString& source, const QString& dist) {
  ensureDir(dist);
  return QFile(source).copy(dist);
}

std::list<std::string> Util::toStdStringList(const QStringList& qStrList) {
  std::list<std::string> list;
  foreach (QString str, qStrList) {
    std::string path = str.toUtf8().constData();
    list.push_back(path);
  }

  return list;
}

QKeySequence Util::toSequence(const QString& aStr) {
  QString str = aStr;
#ifdef Q_OS_MAC
  replace(str, "ctrl|control", "meta");
  replace(str, "cmd|command", "ctrl");
  replace(str, "opt|option", "alt");
#endif

  replace(str, "enter", "return");

  return str;
}

QString Util::toString(const QKeySequence& keySeq) {
  QString str = keySeq.toString().toLower();

#ifdef Q_OS_MAC
  replace(str, "ctrl", "cmd");
  replace(str, "meta", "ctrl");
  replace(str, "alt", "opt");
#endif

  replace(str, "return", "enter");
  return str;
}

}  // namespace core
