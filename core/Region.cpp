#include "Region.h"

namespace core {

bool Region::fullyCovers(const Region& other) const {
  return contains(other.begin()) && other.end() <= end();
}

bool Region::contains(int point) const {
  return begin() <= point && point <= end();
}

void Region::adjust(int pos, int delta) {
  if (m_begin >= pos) {
    m_begin += delta;
  } else {
    int diff = pos + delta - m_begin;
    if (diff < 0) {
      m_begin += diff;
    }
  }

  if (m_end >= pos) {
    m_end += delta;
  } else {
    int diff = pos + delta - m_end;
    if (diff < 0) {
      m_end += diff;
    }
  }
}

bool Region::intersects(const Region& other) const {
  //  return end() > other.begin() && begion() <= other.end();
  return intersection(other).length() > 0;
}

Region Region::intersection(const Region& other) const {
  if (!contains(other.begin()) && !other.contains(begin())) {
    return Region();
  }

  return Region(qMax(begin(), other.begin()), qMin(end(), other.end()));
}

Region Region::sum(const Region &other) const
{
  return Region(qMin(begin(), other.begin()), qMax(end(), other.end()));
}

void Region::setBegin(int p) {
  if (p <= m_end) {
    m_begin = p;
  } else {
    qWarning("%d > end", p);
  }
  Q_ASSERT(m_begin <= m_end);
}

void Region::setEnd(int p) {
  if (p >= m_begin) {
    m_end = p;
  } else {
    qWarning("%d < begin", p);
  }
  Q_ASSERT(m_begin <= m_end);
}

int Region::length() const {
  return end() - begin();
}

QString Region::toString() const {
  return QString("[%1 - %2)").arg(m_begin).arg(m_end);
}

}  // namespace core
