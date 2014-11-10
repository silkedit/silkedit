#include <QDebug>

#include "ContextService.h"

void ContextService::add(const QString& key, CREATION_METHOD creationMethod) {
  m_contexts[key] = std::move(creationMethod);
}

std::shared_ptr<IContext> ContextService::tryCreate(const QString& key,
                                                    Operator op,
                                                    const QString& operand) {
  if (m_contexts.find(key) == m_contexts.end())
    return nullptr;

  return m_contexts.at(key)(op, operand);
}
