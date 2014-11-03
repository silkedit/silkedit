#pragma once

#include <QString>

namespace std {
  template <>
  struct hash<QString> {
    size_t operator () (const QString& str) const {
      return qHash(str);
    }
  };
}
