#pragma once

#include <unordered_map>
#include <memory>

#include "macros.h"
#include "stlSpecialization.h"
#include "IContext.h"
#include "Singleton.h"

class QString;

class ContextManager : public Singleton<ContextManager> {
  DISABLE_COPY_AND_MOVE(ContextManager)

 public:
  ~ContextManager() = default;

  void add(const QString& key, std::unique_ptr<IContextCreator> creator);
  void remove(const QString& key);
  std::shared_ptr<IContext> tryCreate(const QString& key, Operator op, const QString& operand);

 private:
  friend class Singleton<ContextManager>;
  ContextManager() = default;

  std::unordered_map<QString, std::unique_ptr<IContextCreator>> m_contexts;
};
