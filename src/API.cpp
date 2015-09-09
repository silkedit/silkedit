#include <string>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>

#include "API.h"
#include "Window.h"
#include "CommandManager.h"
#include "commands/PluginCommand.h"
#include "PluginManager.h"
#include "TextEditView.h"
#include "SilkApp.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "ProjectManager.h"
#include "KeymapManager.h"
#include "util.h"
#include "Context.h"
#include "PluginContext.h"
#include "modifiers.h"
#include "ConfigManager.h"
#include "Session.h"
#include "util/DialogUtils.h"
#include "InputDialog.h"

std::unordered_map<QString, std::function<void(msgpack::object)>> API::s_notifyFunctions;
std::unordered_map<QString, std::function<void(msgpack::rpc::msgid_t, msgpack::object)>>
    API::s_requestFunctions;

void API::init() {
  s_notifyFunctions.insert(std::make_pair("alert", &alert));
  s_notifyFunctions.insert(std::make_pair("loadMenu", &loadMenu));
  s_notifyFunctions.insert(std::make_pair("registerCommands", &registerCommands));
  s_notifyFunctions.insert(std::make_pair("open", &open));
  s_notifyFunctions.insert(std::make_pair("registerContext", &registerContext));
  s_notifyFunctions.insert(std::make_pair("unregisterContext", &unregisterContext));
  s_notifyFunctions.insert(std::make_pair("dispatchCommand", &dispatchCommand));
  s_notifyFunctions.insert(std::make_pair("setFont", &setFont));

  s_requestFunctions.insert(std::make_pair("activeView", &activeView));
  s_requestFunctions.insert(std::make_pair("activeTabView", &activeTabView));
  s_requestFunctions.insert(std::make_pair("activeTabViewGroup", &activeTabViewGroup));
  s_requestFunctions.insert(std::make_pair("activeWindow", &activeWindow));
  s_requestFunctions.insert(std::make_pair("windows", &windows));
  s_requestFunctions.insert(
      std::make_pair("showFileAndDirectoryDialog", &showFileAndDirectoryDialog));
  s_requestFunctions.insert(std::make_pair("showFilesDialog", &showFilesDialog));
  s_requestFunctions.insert(std::make_pair("showDirectoryDialog", &showDirectoryDialog));
  s_requestFunctions.insert(std::make_pair("getConfig", &getConfig));
  s_requestFunctions.insert(std::make_pair("version", &version));
  s_requestFunctions.insert(std::make_pair("showFontDialog", &showFontDialog));
  s_requestFunctions.insert(std::make_pair("showInputDialog", &showInputDialog));
  s_requestFunctions.insert(std::make_pair("newInputDialog", &newInputDialog));
}

void API::hideActiveFindReplacePanel() {
  if (Window* window = SilkApp::activeWindow()) {
    window->hideFindReplacePanel();
  }
}

void API::call(const QString& method, const msgpack::object& obj) {
  if (s_notifyFunctions.count(method) != 0) {
    s_notifyFunctions.at(method)(obj);
  } else {
    qWarning("%s is not supported", qPrintable(method));
  }
}

void API::call(const QString& method, msgpack::rpc::msgid_t msgId, const msgpack::object& obj) {
  if (s_requestFunctions.count(method) != 0) {
    s_requestFunctions.at(method)(msgId, obj);
  } else {
    qWarning("%s is not supported", qPrintable(method));
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
  int numArgs = obj.via.array.size;
  if (numArgs == 2) {
    msgpack::type::tuple<std::string, std::string> params;
    obj.convert(&params);
    std::string pkgName = std::get<0>(params);
    std::string ymlPath = std::get<1>(params);
    Window::loadMenu(pkgName, ymlPath);
  } else {
    qWarning("invalid arguments. numArgs: %d", numArgs);
  }
}

void API::registerCommands(msgpack::object obj) {
  msgpack::type::tuple<std::vector<std::string>> params;
  obj.convert(&params);
  std::vector<std::string> commands = std::get<0>(params);
  for (std::string& cmd : commands) {
    qDebug("command: %s", cmd.c_str());
    CommandManager::add(
        std::unique_ptr<ICommand>(new PluginCommand(QString::fromUtf8(cmd.c_str()))));
  }
}

void API::registerContext(msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 1) {
    msgpack::type::tuple<std::string> params;
    obj.convert(&params);
    QString context = QString::fromUtf8(std::get<0>(params).c_str());
    Context::add(context, std::move(std::unique_ptr<IContext>(new PluginContext(context))));
  }
}

void API::unregisterContext(msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 1) {
    msgpack::type::tuple<std::string> params;
    obj.convert(&params);
    QString context = QString::fromUtf8(std::get<0>(params).c_str());
    Context::remove(context);
  }
}

void API::activeView(msgpack::rpc::msgid_t msgId, msgpack::object) {
  TextEditView* editView = SilkApp::activeEditView();
  if (editView) {
    PluginManager::singleton().sendResponse(editView->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::activeTabView(msgpack::rpc::msgid_t msgId, msgpack::object) {
  TabView* tabView = SilkApp::activeTabView();
  if (tabView) {
    PluginManager::singleton().sendResponse(tabView->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::activeTabViewGroup(msgpack::rpc::msgid_t msgId, msgpack::object) {
  TabViewGroup* tabViewGroup = SilkApp::activeTabViewGroup();
  if (tabViewGroup) {
    PluginManager::singleton().sendResponse(tabViewGroup->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::activeWindow(msgpack::rpc::msgid_t msgId, msgpack::object) {
  Window* window = SilkApp::activeWindow();
  if (window) {
    PluginManager::singleton().sendResponse(window->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::showFileAndDirectoryDialog(msgpack::rpc::msgid_t msgId, msgpack::object obj) {
  showDialogImpl(msgId, obj, DialogUtils::MODE::FileAndDirectory);
}

void API::showFilesDialog(msgpack::rpc::msgid_t msgId, msgpack::object obj) {
  showDialogImpl(msgId, obj, DialogUtils::MODE::Files);
}

void API::showDirectoryDialog(msgpack::rpc::msgid_t msgId, msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 1) {
    msgpack::type::tuple<std::string> params;
    obj.convert(&params);
    QString caption = QString::fromUtf8(std::get<0>(params).c_str());
    std::list<std::string> paths = DialogUtils::showDialog(caption, DialogUtils::MODE::Directory);
    for (std::string& path : paths) {
      qDebug() << path.c_str();
    }
    if (paths.empty()) {
      PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
    } else {
      PluginManager::singleton().sendResponse(paths.front(), msgpack::type::nil(), msgId);
    }
  }
}

void API::showDialogImpl(msgpack::rpc::msgid_t msgId,
                         const msgpack::object& obj,
                         DialogUtils::MODE mode) {
  int numArgs = obj.via.array.size;
  if (numArgs == 1) {
    msgpack::type::tuple<std::string> params;
    obj.convert(&params);
    QString caption = QString::fromUtf8(std::get<0>(params).c_str());
    std::list<std::string> paths = DialogUtils::showDialog(caption, mode);
    for (std::string& path : paths) {
      qDebug() << path.c_str();
    }
    PluginManager::singleton().sendResponse(paths, msgpack::type::nil(), msgId);
  }
}

void API::windows(msgpack::rpc::msgid_t msgId, msgpack::object) {
  std::vector<int> ids(Window::windows().size());
  for (Window* w : Window::windows()) {
    ids.push_back(w->id());
  }
  PluginManager::singleton().sendResponse(ids, msgpack::type::nil(), msgId);
}

void API::getConfig(msgpack::rpc::msgid_t msgId, msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 1) {
    std::tuple<std::string> params;
    obj.convert(&params);
    std::string nameStr = std::get<0>(params);
    QString name = QString::fromUtf8(nameStr.c_str());
    if (ConfigManager::contains(name)) {
      QString value = ConfigManager::strValue(name);
      std::string valueStr = value.toUtf8().constData();
      PluginManager::singleton().sendResponse(valueStr, msgpack::type::nil(), msgId);
    } else {
      PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
    }
  } else {
    qWarning("invalid arguments. numArgs: %d", numArgs);
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::version(msgpack::rpc::msgid_t msgId, msgpack::object) {
  std::string version = SilkApp::applicationVersion().toUtf8().constData();
  PluginManager::singleton().sendResponse(version, msgpack::type::nil(), msgId);
}

void API::showFontDialog(msgpack::rpc::msgid_t msgId, msgpack::object) {
  bool ok;
  // If the user clicks OK, the selected font is returned. If the user clicks Cancel, the initial
  // font is returned.
  QFont font = QFontDialog::getFont(&ok, Session::singleton().font());
  auto fontParams = std::make_tuple(font.family().toUtf8().constData(), font.pointSize());
  if (ok) {
    PluginManager::singleton().sendResponse(fontParams, msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::showInputDialog(msgpack::rpc::msgid_t msgId, msgpack::v1::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 3) {
    std::tuple<std::string, std::string, std::string> params;
    obj.convert(&params);
    std::string titleStr = std::get<0>(params);
    QString title = QString::fromUtf8(titleStr.c_str());
    std::string labelStr = std::get<1>(params);
    QString label = QString::fromUtf8(labelStr.c_str());
    std::string initialTextStr = std::get<2>(params);
    QString initialText = QString::fromUtf8(initialTextStr.c_str());
    int result;

    InputDialog dialog;
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setTextValue(initialText);
    result = dialog.exec();

    if (result == QDialog::Accepted) {
      QString text = dialog.textValue();
      std::string textStr = text.toUtf8().constData();
      PluginManager::singleton().sendResponse(textStr, msgpack::type::nil(), msgId);
    } else {
      PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
    }
  } else {
    qWarning("invalid arguments. numArgs: %d", numArgs);
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::newInputDialog(msgpack::rpc::msgid_t msgId, msgpack::v1::object) {
  InputDialog* dialog = new InputDialog();
  PluginManager::singleton().sendResponse(dialog->id(), msgpack::type::nil(), msgId);
}

void API::open(msgpack::object obj) {
  msgpack::type::tuple<std::string> params;
  obj.convert(&params);
  QString path = QDir::fromNativeSeparators(QString::fromUtf8(std::get<0>(params).c_str()));
  QFileInfo info(path);
  if (info.isFile()) {
    DocumentManager::open(path);
  } else if (info.isDir()) {
    ProjectManager::open(path);
  } else {
    qWarning("%s is neither file nor directory.", qPrintable(path));
  }
}

void API::dispatchCommand(msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 7) {
    msgpack::type::tuple<std::string, std::string, bool, bool, bool, bool, bool> params;
    obj.convert(&params);
    std::string typeStr = std::get<0>(params);
    QEvent::Type type;
    if (typeStr == "keypress") {
      type = QEvent::KeyPress;
    } else if (typeStr == "keyup") {
      type = QEvent::KeyRelease;
    } else {
      qWarning("invalid key event type: %s", typeStr.c_str());
      return;
    }

    QString key = QString::fromUtf8(std::get<1>(params).c_str());
    bool autorep = std::get<2>(params);
    bool altKey = std::get<3>(params);
    bool ctrlKey = std::get<4>(params);
    bool metaKey = std::get<5>(params);
    bool shiftKey = std::get<6>(params);
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    if (altKey) {
      modifiers |= Silk::AltModifier;
    }
    if (ctrlKey) {
      modifiers |= Silk::ControlModifier;
    }
    if (metaKey) {
      modifiers |= Silk::MetaModifier;
    }
    if (shiftKey) {
      modifiers |= Silk::ShiftModifier;
    }

    KeymapManager::singleton().dispatch(
        new QKeyEvent(type, QKeySequence(key)[0], modifiers, key, autorep));
  } else {
    qWarning("invalid arguments. numArgs: %d", numArgs);
  }
}

void API::setFont(msgpack::object obj) {
  int numArgs = obj.via.array.size;
  if (numArgs == 2) {
    msgpack::type::tuple<std::string, int> params;
    obj.convert(&params);
    std::string family = std::get<0>(params);
    int size = std::get<1>(params);
    QFont font(QString::fromUtf8(family.c_str()), size);
    Session::singleton().setFont(font);
  } else {
    qWarning("invalid arguments. numArgs: %d", numArgs);
  }
}
