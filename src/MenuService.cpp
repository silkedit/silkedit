#include <yaml-cpp/yaml.h>

#include "MenuService.h"
#include "MenuBar.h"
#include "CommandAction.h"

namespace {
QAction* findAction(QList<QAction*> actions, const QString& label) {
  foreach(QAction * action, actions) {
    if (action->text().replace("&", "") == label) {
      return action;
    }
  }

  return nullptr;
}

void parseMenuNode(QWidget* parent, YAML::Node menuNode) {
  if (!menuNode.IsSequence()) {
    qWarning("menuNode must be a sequence.");
    return;
  }

  for (auto it = menuNode.begin(); it != menuNode.end(); it++) {
    YAML::Node node = *it;
    if (!node.IsMap()) {
      qWarning("menu item must be a map");
      continue;
    }

    YAML::Node labelNode = node["label"];
    if (labelNode.IsDefined() && labelNode.IsScalar()) {
      QString label = QString::fromUtf8(labelNode.as<std::string>().c_str());
      YAML::Node commandNode = node["command"];
      YAML::Node submenuNode = node["submenu"];
      if (submenuNode.IsDefined()) {
        QMenu* currentMenu;
        if (QAction* action = findAction(parent->actions(), label)) {
          currentMenu = action->menu();
        } else {
          currentMenu = new QMenu(label, parent);
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            menuBar->addMenu(currentMenu);
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            parentMenu->addMenu(currentMenu);
          }
        }
        parseMenuNode(currentMenu, submenuNode);
      } else if (commandNode.IsDefined()) {
        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        auto commandAction = new CommandAction(label, command);
        if (!findAction(parent->actions(), label)) {
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            menuBar->addAction(commandAction);
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            parentMenu->addAction(commandAction);
          }
        } else {
          qWarning("%s already exists", qPrintable(label));
        }
      }
    }
  }
}
}

MenuBar* MenuService::m_menuBar = nullptr;

void MenuService::init() { m_menuBar = new MenuBar(nullptr); }

void MenuService::loadMenu(const std::string& ymlPath) {
  qDebug("Start loading: %s", ymlPath.c_str());
  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath);
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return;
    }

    YAML::Node menuNode = rootNode["menu"];
    parseMenuNode(m_menuBar, menuNode);
  }
  catch (const YAML::ParserException& ex) {
    qWarning("Unable to load %s. Cause: %s", ymlPath.c_str(), ex.what());
  }
}
