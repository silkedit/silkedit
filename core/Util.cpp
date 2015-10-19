#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include "Util.h"

namespace core {

// This function is same as sort.Search in golang
// Search uses binary search to find and return the smallest index i in [0, n) at which f(i) is
// true, assuming that on the range [0, n), f(i) == true implies f(i+1) == true. That is, Search
// requires that f is false for some (possibly empty) prefix of the input range [0, n) and then true
// for the (possibly empty) remainder; Search returns the first true index. If there is no such
// index, Search returns n. (Note that the "not found" return value is not -1 as in, for instance,
// strings.Index). Search calls f(i) only for i in the range [0, n).
int Util::binarySearch(int last, std::function<bool(int)> fn) {
  int low = 0;
  int high = last;
  while (low < high) {
    int mid = (low + high) / 2;
    //    qDebug("low: %d, high: %d, mid: %d", low, high, mid);
    if (fn(mid)) {
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

}  // namespace core
