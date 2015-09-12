#include "Context.h"
#include "core/OSContext.h"
#include "PluginContext.h"

using core::IContext;
using core::Operator;
using core::OSContext;

std::unordered_map<QString, std::unique_ptr<IContext>> Context::s_contexts;

void Context::init() {
  // register default contexts
  add(OSContext::name, std::move(std::unique_ptr<IContext>(new OSContext())));
}

void Context::add(const QString& key, std::unique_ptr<IContext> context) {
  s_contexts[key] = std::move(context);
}

void Context::remove(const QString& key) {
  s_contexts.erase(key);
}

Context::Context(const QString& key, Operator op, const QString& value)
    : m_key(key), m_op(op), m_value(value) {
}

bool Context::isSatisfied() {
  if (s_contexts.find(m_key) == s_contexts.end())
    return false;

  return s_contexts.at(m_key)->isSatisfied(m_op, m_value);
}

bool Context::operator==(const Context& other) const {
  return this->m_key == other.m_key && this->m_op == other.m_op && this->m_value == other.m_value;
}
