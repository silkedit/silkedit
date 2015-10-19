#include "PluginContext.h"
#include "PluginManager.h"

using core::Operator;

PluginContext::PluginContext(const QString& key) : m_key(key) {
}

bool PluginContext::isSatisfied(Operator op, const QString& operand) {
  return PluginManager::singleton().askExternalContext(m_key, op, operand);
}

QString PluginContext::key() {
  throw std::runtime_error("this method should not be called");
}
