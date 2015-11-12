#include "PluginCondition.h"
#include "PluginManager.h"

using core::Operator;

PluginCondition::PluginCondition(const QString& key) : m_key(key) {}

bool PluginCondition::isSatisfied(Operator op, const QString& operand) {
  return PluginManager::singleton().askExternalCondition(m_key, op, operand);
}

QString PluginCondition::key() {
  throw std::runtime_error("this method should not be called");
}
