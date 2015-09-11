#pragma once

#include <sstream>
#include <unordered_map>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "core/macros.h"
#include "CommandArgument.h"

namespace {
std::string toString(const CommandArgument& arg) {
  std::stringstream ss;
  for (std::pair<std::string, std::string> pair : arg) {
    ss << pair.first.c_str() << ": " << pair.second.c_str() << std::endl;
  }

  return ss.str();
}
}

namespace core {

class ICommand {
  DISABLE_COPY_AND_MOVE(ICommand)
 public:
  explicit ICommand(QString name) : m_name(name) {}
  virtual ~ICommand() = default;

  inline void run(const CommandArgument& args, int repeat = 1) {
    qDebug() << "Start command: " << m_name << "args: " << toString(args).c_str()
             << "repeat: " << repeat;
    doRun(args, repeat);
    qDebug() << "End command: " << m_name;
  }

  inline QString name() { return m_name; }

 private:
  virtual void doRun(const CommandArgument& args, int repeat = 1) = 0;

  QString m_name;
};

}  // namespace core
