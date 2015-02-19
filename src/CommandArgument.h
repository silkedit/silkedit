#pragma once

#include <unordered_map>
#include <QVariant>
#include <QDebug>

#include "macros.h"
#include "stlSpecialization.h"

class QString;

class CommandArgument {
  typedef std::unordered_map<QString, QVariant> ARGS;

 public:
  CommandArgument() = default;
  explicit CommandArgument(ARGS args);
  ~CommandArgument() = default;
  DEFAULT_COPY_AND_MOVE(CommandArgument)

  template <typename T>
  bool contains(const QString& key) const {
    if (m_args.find(key) == m_args.end()) {
      return false;
    }

    QVariant argVar = m_args.at(key);
    if (!argVar.canConvert<T>()) {
      return false;
    }

    return true;
  }

  template <typename T>
  inline T value(const QString& key) const {
    QVariant argVar = m_args.at(key);
    return argVar.value<T>();
  }

  friend QDebug operator<<(QDebug dbg, const CommandArgument& arg) {
    for (std::pair<QString, QVariant> pair : arg.m_args) {
      dbg.nospace() << pair.first << ": " << pair.second;
    }

    return dbg.space();
  }

 private:
  ARGS m_args;
};
