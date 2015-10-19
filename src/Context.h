#pragma once

#include <memory>
#include <unordered_map>
#include <QString>

#include "core/IContext.h"
#include "core/stlSpecialization.h"

class Context {
 public:
  static void init();
  static void add(const QString& key, std::unique_ptr<core::IContext> context);
  static void remove(const QString& key);

  QString m_key;
  core::Operator m_op;
  QString m_value;

  Context(const QString& key, core::Operator op, const QString& value);
  bool isSatisfied();

  /**
   * @brief check if context is static (e.g., os == mac)
   * @return
   */
  bool isStatic();
  bool operator==(const Context& other) const;

  bool operator!=(const Context& other) const { return !(*this == other); }

 private:
  static std::unordered_map<QString, std::unique_ptr<core::IContext>> s_contexts;
};
