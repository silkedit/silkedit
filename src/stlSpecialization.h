#pragma once

#include <QString>
#include <QKeySequence>
#include <QHash>

namespace std {

template <>
struct hash<QString> {
  size_t operator()(const QString& str) const { return qHash(str); }
};

template <>
struct hash<QKeySequence> {
  size_t operator()(const QKeySequence& seq) const {
    uint hash = 0;
    for (int i = 0; i < seq.count(); i++) {
      hash += qHash(seq[i]);
    }
    return hash;
  }
};
}
