#pragma once

#include <unordered_map>
#include <memory>

#include "macros.h"
#include "stlSpecialization.h"
#include "IContext.h"
#include "Singleton.h"
#include "DefaultContext.h"

class QString;

class ContextService : public Singleton<ContextService> {
  DISABLE_COPY_AND_MOVE(ContextService)

 public:
  ~ContextService() = default;

  void add(const QString& key, std::unique_ptr<IContextCreator> creator);
  std::shared_ptr<IContext> tryCreate(const QString& key, Operator op, const QString& operand);
  std::shared_ptr<DefaultContext> createDefault() {
    return std::shared_ptr<DefaultContext>(new DefaultContext());
  }

 private:
  friend class Singleton<ContextService>;
  ContextService() = default;

  std::unordered_map<QString, std::unique_ptr<IContextCreator>> m_contexts;
};
