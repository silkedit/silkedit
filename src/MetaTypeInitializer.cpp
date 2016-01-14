#include "MetaTypeInitializer.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "Window.h"
#include "Helper.h"
#include "StatusBar.h"
#include "TextEditView.h"
#include "core/qdeclare_metatype.h"
#include "core/Condition.h"

void MetaTypeInitializer::init()
{
  qRegisterMetaType<boost::optional<QString>>();
  qRegisterMetaType<TabView*>();
  qRegisterMetaType<TextEditView*>();
  qRegisterMetaType<TabViewGroup*>();
  qRegisterMetaType<Window*>();
  qRegisterMetaType<StatusBar*>();
  qRegisterMetaType<CommandEventFilterResult>();
  qRegisterMetaType<std::string>();
  qRegisterMetaType<QMetaProperty>();
  qRegisterMetaType<QPushButton*>();
  qRegisterMetaType<QLayout*>();
  qRegisterMetaType<core::Condition::Operator>();
}

