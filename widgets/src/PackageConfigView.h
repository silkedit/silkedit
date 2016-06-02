#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "CustomWidget.h"
#include "Filtering.h"
#include "core/macros.h"

namespace core {
struct ConfigDefinition;
}

class PackageConfigView : public CustomWidget, public Filtering {
  Q_OBJECT
  DISABLE_COPY(PackageConfigView)

 public:
  PackageConfigView(const QList<core::ConfigDefinition>& defs);
  ~PackageConfigView() = default;
  DEFAULT_MOVE(PackageConfigView)

 private:
};

class ConfigCheckBox : public QCheckBox {
 public:
  ConfigCheckBox(const core::ConfigDefinition& def, QWidget* parent = nullptr);
  ~ConfigCheckBox() = default;
};

class ConfigLineEdit : public QLineEdit {
 public:
  ConfigLineEdit(const core::ConfigDefinition& def, QWidget* parent = nullptr);
  ~ConfigLineEdit() = default;
};

class ConfigSpinBox : public QSpinBox {
 public:
  ConfigSpinBox(const core::ConfigDefinition& def, QWidget* parent = nullptr);
  ~ConfigSpinBox() = default;
};

class ConfigDoubleSpinBox : public QDoubleSpinBox {
 public:
  ConfigDoubleSpinBox(const core::ConfigDefinition& def, QWidget* parent = nullptr);
  ~ConfigDoubleSpinBox() = default;
};
