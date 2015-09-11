#pragma once

#include "macros.h"

namespace core {

class IOperator {
  DISABLE_COPY(ICommand)
 public:
  ICommand(QString name) : m_name(name) {}
  ICommand(ICommand&&) = default;
  virtual ~ICommand() = default;

  ICommand& operator=(ICommand&&) = default;

  inline void run(const std::unordered_map<QString, QVariant>& args) {
    qDebug() << "Start command: " << m_name;
    doRun(args);
    qDebug() << "End command: " << m_name;
  }

  inline QString name() { return m_name; }

 private:
  virtual void doRun(const std::unordered_map<QString, QVariant>& args) = 0;

  QString m_name;
};

}  // namespace core
