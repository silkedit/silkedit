#pragma once

#include <sstream>
#include <unordered_map>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "core/macros.h"
#include "core/CommandArgument.h"

class ICommand {
  DISABLE_COPY_AND_MOVE(ICommand)
 public:
  explicit ICommand(const QString& name, const QString& description = "")
      : m_name(name), m_description(description) {}
  virtual ~ICommand() = default;

  inline void run(const CommandArgument& args, int repeat = 1) {
    doRun(args, repeat);
  }

  QString name() { return m_name; }
  QString description() { return m_description; }

 private:
  virtual void doRun(const CommandArgument& args, int repeat = 1) = 0;

  QString m_name;
  QString m_description;
};
