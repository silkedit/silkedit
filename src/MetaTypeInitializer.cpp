#include "MetaTypeInitializer.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "Window.h"
#include "StatusBar.h"
#include "TextEditView.h"
#include "core/Condition.h"
#include "core/FunctionInfo.h"
#include "core/Url.h"
#include "core/Font.h"
#include "core/QKeyEventWrap.h"

void MetaTypeInitializer::init()
{
  qRegisterMetaType<TabView*>();
  qRegisterMetaType<QLinkedList<TabView*>>();
  qRegisterMetaType<TextEditView*>();
  qRegisterMetaType<TabViewGroup*>();
  qRegisterMetaType<Window*>();
  qRegisterMetaType<StatusBar*>();
  qRegisterMetaType<QPushButton*>();
  qRegisterMetaType<QLayout*>();
  qRegisterMetaType<core::Condition::Operator>();
  qRegisterMetaType<core::FunctionInfo>("FunctionInfo");
  qRegisterMetaType<core::FunctionInfo>("core::FunctionInfo");
  qRegisterMetaType<core::Url*>("Url*");
  qRegisterMetaType<core::Url*>("core::Url*");
  qRegisterMetaType<core::Url::ParsingMode>("ParsingMode");
  qRegisterMetaType<core::Font*>("Font*");
  qRegisterMetaType<core::Font*>("core::Font*");
  qRegisterMetaType<core::QKeyEventWrap*>("QKeyEventWrap*");
  qRegisterMetaType<core::QKeyEventWrap*>("core::QKeyEventWrap*");
}

