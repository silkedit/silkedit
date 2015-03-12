#include "Context.h"
#include "OSContext.h"
#include "ModeContext.h"

std::unordered_map<QString, IContext*> Context::s_contexts;

void Context::init() {
  // register default contexts
  add(OSContext::name, &OSContext::singleton());
  add(ModeContext::name, &ModeContext::singleton());
}

void Context::add(const QString& key, IContext* context) {
  s_contexts[key] = context;
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
