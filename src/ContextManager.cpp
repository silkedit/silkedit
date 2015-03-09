#include <QDebug>

#include "ContextManager.h"

void ContextManager::add(const QString& key, std::unique_ptr<IContextCreator> creationMethod) {
  m_contexts[key] = std::move(creationMethod);
}

void ContextManager::remove(const QString& key) {
  m_contexts.erase(key);
}

std::shared_ptr<IContext> ContextManager::tryCreate(const QString& key,
                                                    Operator op,
                                                    const QString& operand) {
  if (m_contexts.find(key) == m_contexts.end())
    return nullptr;

  return m_contexts.at(key)->create(op, operand);
}
