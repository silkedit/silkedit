#pragma once

#include "ICondition.h"
#include "macros.h"

namespace core {

class OSCondition : public ICondition {
  DISABLE_COPY(OSCondition)

 public:
  static const QString name;

  OSCondition() = default;
  ~OSCondition() = default;
  DEFAULT_MOVE(OSCondition)

  bool isStatic() override { return true; }

 private:
  QString key() override;
};

class OnMacCondition : public ICondition {
  DISABLE_COPY(OnMacCondition)

 public:
  static const QString name;

  OnMacCondition() = default;
  ~OnMacCondition() = default;
  DEFAULT_MOVE(OnMacCondition)

  bool isStatic() override { return true; }

 private:
  QString key() override;
};

class OnWindowsCondition : public ICondition {
  DISABLE_COPY(OnWindowsCondition)

 public:
  static const QString name;

  OnWindowsCondition() = default;
  ~OnWindowsCondition() = default;
  DEFAULT_MOVE(OnWindowsCondition)

  bool isStatic() override { return true; }

 private:
  QString key() override;
};

}  // namespace core
