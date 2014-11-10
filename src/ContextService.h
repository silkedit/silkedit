#pragma once

#include <unordered_map>
#include <memory>

#include "IContext.h"
#include "macros.h"
#include "singleton.h"
#include "stlSpecialization.h"

class QString;

class ContextService : public Singleton<ContextService> {
  DISABLE_COPY_AND_MOVE(ContextService)

 public:
  ~ContextService() = default;

  void add(const QString& key, CREATION_METHOD creationMethod);
  std::shared_ptr<IContext> tryCreate(const QString& key, Operator op, const QString& operand);

 private:
  friend class Singleton<ContextService>;
  ContextService() = default;

  std::unordered_map<QString, CREATION_METHOD> m_contexts;
};
