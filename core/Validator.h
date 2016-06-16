#pragma once

#include <v8.h>
#include <QValidator>

#include "macros.h"

namespace core {

struct FunctionInfo;

class Validator : public QValidator {
  Q_OBJECT

 public:
  // redefined enums
  enum State {
    Invalid = QValidator::State::Invalid,
    Intermediate = QValidator::State::Intermediate,
    Acceptable = QValidator::State::Acceptable
  };
  Q_ENUM(State)

  Q_INVOKABLE explicit Validator(const core::FunctionInfo& fn, QObject* parent = nullptr);
  ~Validator() = default;
  DEFAULT_MOVE(Validator)
  DISABLE_COPY(Validator)

  QValidator::State validate(QString& input, int& pos) const override;

 private:
  v8::UniquePersistent<v8::Function> m_validateFn;
};

}  // namespace core
