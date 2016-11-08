#pragma once

#include <QStringList>

namespace core {

class SquirrelHandler {
 public:
  static QStringList handleArguments(QStringList arguments);

  SquirrelHandler() = delete;
  ~SquirrelHandler() = delete;
};

}  // namespace core
