#include <QList>

#include "MetaTypeInitializer.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "Window.h"
#include "StatusBar.h"
#include "Console.h"
#include "TextEditView.h"
#include "core/Condition.h"
#include "core/FunctionInfo.h"
#include "core/Url.h"
#include "core/Font.h"
#include "core/QKeyEventWrap.h"
#include "core/TextCursor.h"
#include "core/TextBlock.h"
#include "core/LanguageParser.h"
#include "core/Region.h"
#include "core/SyntaxHighlighter.h"

void MetaTypeInitializer::init()
{
  qRegisterMetaType<TabView*>();
  qRegisterMetaType<QLinkedList<TabView*>>();
  qRegisterMetaType<TextEditView*>();
  qRegisterMetaType<TabViewGroup*>();
  qRegisterMetaType<Window*>();
  qRegisterMetaType<StatusBar*>();
  qRegisterMetaType<Console*>();
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
  qRegisterMetaType<core::TextCursor*>("TextCursor*");
  qRegisterMetaType<core::TextCursor*>("core::TextCursor*");
  qRegisterMetaType<core::TextCursor::MoveMode>("MoveMode");
  qRegisterMetaType<core::TextCursor::MoveOperation>("MoveOperation");
  qRegisterMetaType<core::TextCursor::SelectionType>("SelectionType");
  qRegisterMetaType<core::TextBlock*>("TextBlock*");
  qRegisterMetaType<core::TextBlock*>("core::TextBlock*");
  qRegisterMetaType<QList<core::Node>>("QList<Node>");
  qRegisterMetaType<QList<core::Node>>("QList<core::Node>");
  qRegisterMetaType<core::RootNode>("RootNode");
  qRegisterMetaType<core::RootNode>("core::RootNode");
  qRegisterMetaType<core::LanguageParser>("LanguageParser");
  qRegisterMetaType<core::LanguageParser>("core::LanguageParser");
  qRegisterMetaType<core::Region>("Region");
  qRegisterMetaType<core::Region>("core::Region");
  qRegisterMetaType<core::SyntaxHighlighter*>("SyntaxHighlighter*");
  qRegisterMetaType<core::SyntaxHighlighter*>("core::SyntaxHighlighter*");
}

