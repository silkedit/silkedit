#pragma once

#include <v8.h>
#include <unordered_map>
#include <memory>
#include <QObject>

#include "stlSpecialization.h"
#include "macros.h"

class QString;

namespace core {

class Condition : public QObject {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Condition)
 public:
  enum Operator {
    EQUALS,
    NOT_EQUALS,
  };
  Q_ENUM(Operator)


  static QString operatorString(Operator op);

  static void Init(v8::Local<v8::Object> exports);
  static void init();
  static bool isStatic(const QString& keyValue);
  static bool isSatisfied(const QString& keyValue, core::Condition::Operator op, const QString& value);

  virtual ~Condition() = default;

  virtual bool isSatisfied(Operator op, const QString& operand);

  /**
   * @brief if condition is static or not
   * @return
   */
  virtual bool isStatic() = 0;

 protected:
  Condition() = default;

 private:
  static std::unordered_map<QString, std::unique_ptr<Condition>> s_conditions;

  static void add(const QString& keyValue, std::unique_ptr<Condition> condition);
  static void remove(const QString& keyValue);
  static bool check(const QString &keyValue, Operator op, const QString& operand);
  static void Add(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Remove(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Check(const v8::FunctionCallbackInfo<v8::Value>& args);

  virtual QString keyValue() = 0;
};

}  // namespace core
