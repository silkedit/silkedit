#pragma once

#include <unordered_map>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "macros.h"
#include "CommandArgument.h"

class ICommand {
  DISABLE_COPY_AND_MOVE(ICommand)
 public:
  explicit ICommand(QString name) : m_name(name) {}
  virtual ~ICommand() = default;

  inline void run(const CommandArgument& args, int repeat = 1) {
    qDebug() << "Start command: " << m_name << "repeat: " << repeat;
    doRun(args, repeat);
    qDebug() << "End command: " << m_name;
  }

  inline QString name() { return m_name; }

 private:
  virtual void doRun(const CommandArgument& args, int repeat = 1) = 0;

  QString m_name;
};
