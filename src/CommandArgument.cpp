#include "CommandArgument.h"

CommandArgument::CommandArgument(ARGS args) : m_args(std::move(args)) {
}

std::unordered_map<std::string, std::string> CommandArgument::args() const {
  std::unordered_map<std::string, std::string> result;
  for (const std::pair<QString, QVariant>& pair : m_args) {
    result.insert(std::make_pair(pair.first.toUtf8().constData(),
                                 pair.second.toString().toUtf8().constData()));
  }

  return result;
}
