#pragma once

#include <memory>
#include <tuple>
#include <QString>
#include <QJsonValue>
#include <QHash>

#include "macros.h"

// Package model class
namespace core {
struct Package {
  DEFAULT_COPY_AND_MOVE(Package)

  static const int ITEM_COUNT = 4;
  static Package fromJson(const QJsonValue& value) { return std::move(Package(value)); }

  QString name;
  QString version;
  QString description;
  QString repositoryUrl;

  explicit Package(const QJsonValue& jsonValue);
  ~Package() = default;

  QStringList validate();
  QString tarballUrl() const;
  QJsonObject toJson() const;
};

inline bool operator==(const Package& e1, const Package& e2) {
  return e1.name == e2.name;
}

inline uint qHash(const Package& key, uint seed) {
  return qHash(key.name, seed);
}
}  // namespace core
