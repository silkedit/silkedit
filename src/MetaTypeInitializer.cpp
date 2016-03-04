#include <QList>
#include <QEvent>

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
#include "core/KeyEvent.h"
#include "core/TextCursor.h"
#include "core/TextBlock.h"
#include "core/LanguageParser.h"
#include "core/Region.h"
#include "core/SyntaxHighlighter.h"
#include "core/TextOption.h"
#include "core/Completer.h"
#include "core/QAbstractItemViewWrap.h"
#include "core/Rect.h"
#include "core/StringListModel.h"
#include "core/ModelIndex.h"
#include "core/ItemSelectionModel.h"
#include "core/QtEnums.h"

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
  qRegisterMetaType<QAbstractItemModel*>();
  qRegisterMetaType<QAbstractItemView*>();
  qRegisterMetaType<QModelIndex>();
  qRegisterMetaType<QItemSelectionModel*>();
  qRegisterMetaType<QEvent*>();
  qRegisterMetaType<QEvent::Type>("QEvent::Type");
  qRegisterMetaType<core::Condition::Operator>();
  qRegisterMetaType<core::FunctionInfo>("core::FunctionInfo");
  qRegisterMetaType<core::Url*>("core::Url*");
  qRegisterMetaType<core::Url::ParsingMode>("ParsingMode");
  qRegisterMetaType<core::Font*>("core::Font*");
  qRegisterMetaType<core::KeyEvent*>("core::KeyEvent*");
  qRegisterMetaType<core::TextCursor*>("core::TextCursor*");
  qRegisterMetaType<core::TextCursor::MoveMode>("MoveMode");
  qRegisterMetaType<core::TextCursor::MoveOperation>("MoveOperation");
  qRegisterMetaType<core::TextCursor::SelectionType>("SelectionType");
  qRegisterMetaType<core::TextBlock*>("core::TextBlock*");
  qRegisterMetaType<QList<core::Node>>("QList<Node>");
  qRegisterMetaType<core::RootNode>("RootNode");
  qRegisterMetaType<core::LanguageParser>("LanguageParser");
  qRegisterMetaType<core::Region>("Region");
  qRegisterMetaType<core::SyntaxHighlighter*>("SyntaxHighlighter*");
  qRegisterMetaType<core::TextOption*>("core::TextOption*");
  qRegisterMetaType<core::TextOption::Flag>("Flag");
  qRegisterMetaType<core::Document*>("core::Document*");
  qRegisterMetaType<core::Completer::CompletionMode>("CompletionMode");
  qRegisterMetaType<core::Completer::ModelSorting>("ModelSorting");
  qRegisterMetaType<core::QAbstractItemViewWrap*>("core::QAbstractItemViewWrap*");
  qRegisterMetaType<core::Rect*>("core::Rect");
  qRegisterMetaType<core::StringListModel*>("core::StringListModel*");
  qRegisterMetaType<core::ModelIndex*>("core::ModelIndex*");
  qRegisterMetaType<core::ItemSelectionModel*>("core::ItemSelectionModel*");
  qRegisterMetaType<core::ItemSelectionModel*>("ItemSelectionModel*");
  qRegisterMetaType<core::ItemSelectionModel::SelectionFlag>("SelectionFlag");
  qRegisterMetaType<core::QtEnums::KeyboardModifier>("KeyboardModifier");
  qRegisterMetaType<core::QtEnums::EventPriority>("EventPriority");
}
