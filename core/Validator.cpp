#include "Validator.h"
#include "V8Util.h"
#include "FunctionInfo.h"

using v8::Local;
using v8::Value;
using v8::Isolate;
using v8::UniquePersistent;
using v8::Function;

namespace core {

const auto& INPUT_KEY = QStringLiteral("input");
const auto& POS_KEY = QStringLiteral("pos");
const auto& STATE_KEY = QStringLiteral("state");

Validator::Validator(const core::FunctionInfo& fn, QObject* parent) : QValidator(parent) {
  m_validateFn.Reset(fn.isolate, fn.fn);
}

QValidator::State Validator::validate(QString& input, int& pos) const {
  Isolate* isolate = Isolate::GetCurrent();

  constexpr int argc = 2;
  Local<Value> argv[argc];
  argv[0] = V8Util::toV8Value(isolate, QVariant(input));
  argv[1] = V8Util::toV8Value(isolate, QVariant(pos));

  Local<Function> fn = m_validateFn.Get(isolate);
  auto resultVar = V8Util::callJSFunc(isolate, fn, v8::Undefined(isolate), argc, argv);

  if (!resultVar.canConvert<QVariantMap>()) {
    return QValidator::State::Invalid;
  }

  const auto& resultMap = resultVar.toMap();
  if (resultMap.contains(INPUT_KEY) && resultMap[INPUT_KEY].canConvert<QString>()) {
    input = resultMap[INPUT_KEY].toString();
  }

  if (resultMap.contains(POS_KEY) && resultMap[POS_KEY].canConvert<int>()) {
    bool ok;
    auto newPos = resultMap[POS_KEY].toInt(&ok);
    if (ok) {
      pos = newPos;
    }
  }

  if (resultMap.contains(STATE_KEY) && resultMap[STATE_KEY].canConvert<int>()) {
    bool ok;
    auto stateInt = resultMap[STATE_KEY].toInt(&ok);
    if (ok) {
      switch (stateInt) {
        case Validator::State::Intermediate:
          return QValidator::State::Intermediate;
        case Validator::State::Acceptable:
          return QValidator::State::Acceptable;
      }
    }
  }

  return QValidator::State::Invalid;
}

}  // namespace core
