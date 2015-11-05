#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>

#include "PackageConfigView.h"
#include "core/ConfigDefinition.h"
#include "core/Config.h"

using core::ConfigDefinition;
using core::Config;

PackageConfigView::PackageConfigView(const QList<core::ConfigDefinition>& defs) {
  QVBoxLayout* rootLayout = new QVBoxLayout;
  for (const ConfigDefinition& def : defs) {
    switch (def.type()) {
      case QVariant::Bool: {
        auto check = new ConfigCheckBox(def);
        addTargetObject(check);
        rootLayout->addWidget(check);
        break;
      }
      case QVariant::String: {
        auto label = new QLabel(def.title + ":");
        auto lineEdit = new ConfigLineEdit(def);
        label->setBuddy(lineEdit);
        addTargetObject(label);
        rootLayout->addWidget(label);
        rootLayout->addWidget(lineEdit);
        break;
      }
      case QVariant::Int: {
        QHBoxLayout* layout = new QHBoxLayout();
        addTargetObject(layout);
        auto label = new QLabel(def.title + ":");
        auto spin = new ConfigSpinBox(def);
        label->setBuddy(spin);
        layout->addWidget(label);
        layout->addWidget(spin);
        layout->addStretch();
        rootLayout->addLayout(layout);
        break;
      }
      case QVariant::Double: {
        QHBoxLayout* layout = new QHBoxLayout();
        addTargetObject(layout);
        auto label = new QLabel(def.title + ":");
        auto spin = new ConfigDoubleSpinBox(def);
        label->setBuddy(spin);
        layout->addWidget(label);
        layout->addWidget(spin);
        layout->addStretch();
        rootLayout->addLayout(layout);
        break;
      }
      default:
        qWarning("typeid %d is not supported", def.type());
        break;
    }
  }

  rootLayout->addStretch();
  setLayout(rootLayout);
}

ConfigCheckBox::ConfigCheckBox(const core::ConfigDefinition& def, QWidget* parent)
    : QCheckBox(def.title, parent) {
  setChecked(Config::singleton().value(def.key, def.defaultValue.toBool()));
  setToolTip(def.description);
  connect(this, &QCheckBox::toggled, this,
          [=](bool checked) { Config::singleton().setValue(def.key, checked); });
}

ConfigLineEdit::ConfigLineEdit(const core::ConfigDefinition& def, QWidget* parent)
    : QLineEdit(parent) {
  setText(Config::singleton().value(def.key, def.defaultValue.toString()));
  setToolTip(def.description);
  connect(this, &ConfigLineEdit::textEdited, this,
          [=](const QString& text) { Config::singleton().setValue(def.key, text); });
}

ConfigSpinBox::ConfigSpinBox(const core::ConfigDefinition& def, QWidget* parent)
    : QSpinBox(parent) {
  setValue(Config::singleton().value(def.key, def.defaultValue.toInt()));
  setToolTip(def.description);
  connect(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          [=](int value) { Config::singleton().setValue(def.key, value); });
}

ConfigDoubleSpinBox::ConfigDoubleSpinBox(const core::ConfigDefinition& def, QWidget* parent)
    : QDoubleSpinBox(parent) {
  setValue(Config::singleton().value(def.key, def.defaultValue.toDouble()));
  setToolTip(def.description);
  connect(this, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
          [=](double value) { Config::singleton().setValue(def.key, value); });
}
