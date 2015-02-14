#include <QApplication>
#include <QMessageBox>

#include "API.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "MainWindow.h"
#include "MenuService.h"
#include "CommandService.h"
#include "commands/PluginCommand.h"
#include "plugin_service/PluginService.h"
#include "TextEditView.h"

std::unordered_map<std::string, std::function<void(msgpack::object)>> API::notifyFunctions;
std::unordered_map<std::string, std::function<void(msgpack::rpc::msgid_t, msgpack::object)>>
    API::requestFunctions;

void API::init() {
  notifyFunctions.insert(std::make_pair("alert", &alert));
  notifyFunctions.insert(std::make_pair("load_menu", &loadMenu));
  notifyFunctions.insert(std::make_pair("register_commands", &registerCommands));

  requestFunctions.insert(std::make_pair("active_view", &getActiveView));
}

TextEditView* API::activeEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return tabView->activeEditView();
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* API::activeTabView(bool createIfNull) {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab(createIfNull);
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* API::activeTabViewGroup() {
  MainWindow* window = activeWindow();
  if (window) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

MainWindow* API::activeWindow() {
  return qobject_cast<MainWindow*>(QApplication::activeWindow());
}

QList<MainWindow*> API::windows() {
  return MainWindow::windows();
}

void API::hideActiveFindReplacePanel() {
  if (MainWindow* window = activeWindow()) {
    window->hideFindReplacePanel();
  }
}

void API::call(const std::string& method, const msgpack::object& obj) {
  if (notifyFunctions.count(method) != 0) {
    notifyFunctions.at(method)(obj);
  } else {
    qWarning("%s is not supported", method.c_str());
  }
}

void API::call(const std::string& method, msgpack::rpc::msgid_t msgId, const msgpack::object& obj) {
  if (requestFunctions.count(method) != 0) {
    requestFunctions.at(method)(msgId, obj);
  } else {
    qWarning("%s is not supported", method.c_str());
  }
}

void API::alert(msgpack::object obj) {
  msgpack::type::tuple<std::string> params;
  obj.convert(&params);
  std::string msg = std::get<0>(params);
  QMessageBox msgBox;
  msgBox.setText(QString::fromUtf8(msg.c_str()));
  msgBox.exec();
}

void API::loadMenu(msgpack::object obj) {
  msgpack::type::tuple<std::string> params;
  obj.convert(&params);
  std::string ymlPath = std::get<0>(params);
  MenuService::loadMenu(ymlPath);
}

void API::registerCommands(msgpack::object obj) {
  msgpack::type::tuple<std::vector<std::string>> params;
  obj.convert(&params);
  std::vector<std::string> commands = std::get<0>(params);
  for (std::string& cmd : commands) {
    qDebug("command: %s", cmd.c_str());
    CommandService::add(
        std::unique_ptr<ICommand>(new PluginCommand(QString::fromUtf8(cmd.c_str()))));
  }
}

void API::getActiveView(msgpack::rpc::msgid_t msgId, msgpack::object ) {
  TextEditView* editView = activeEditView();
  if (editView) {
    PluginService::singleton().sendResponse(editView->id(), msgpack::type::nil(), msgId);
  } else {
    PluginService::singleton().sendResponse(
        msgpack::type::nil(), std::string("active edit view is null"), msgId);
  }
}
