#pragma once

#include <QDebug>
#include <QString>

#include "macros.h"

class Region {
public:
  Region() : m_begin(0), m_end(0) {}
  Region(int begin, int end) {
    m_begin = qMin(begin, end);
    m_end = qMax(begin, end);
  }
  DEFAULT_COPY_AND_MOVE(Region)

  // Returns whether the region fully covers the other region
  bool fullyCovers(const Region& other) const;
  bool contains(int point) const;
  bool isEmpty() { return m_begin == m_end; }
  // Adjusts the region in place for the given position and delta
  void adjust(int pos, int delta);
  bool intersects(const Region& other) const;
  Region intersection(const Region& other) const;

  int begin() const { return m_begin; }
  void setBegin(int p);
  int end() const { return m_end; }
  void setEnd(int p);
  int length() const;

  QString toString() const;

private:
  int m_begin;
  int m_end;

  friend QDebug operator<<(QDebug dbg, const Region& region) {
    dbg.nospace() << region.toString();
    return dbg.space();
  }
};
