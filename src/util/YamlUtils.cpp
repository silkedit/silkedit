#include <stack>
#include <QStringList>
#include <QDebug>
#include <QMenuBar>
#include <QUuid>
#include <QToolbar>
#include <QDir>

#include "YamlUtils.h"
#include "Context.h"
#include "CommandAction.h"
#include "PluginManager.h"
#include "Window.h"
#include "ToolBar.h"

using core::Operator;

namespace {
QAction* findAction(QList<QAction*> actions, const QString& id) {
  if (id.isEmpty())
    return nullptr;

  foreach (QAction* action, actions) {
    Q_ASSERT(action);
    if (action->objectName() == id || (action->menu() && action->menu()->objectName() == id)) {
      return action;
    }
  }

  return nullptr;
}
}

Context* YamlUtils::parseContext(const YAML::Node& contextNode) {
  QString contextStr = QString::fromUtf8(contextNode.as<std::string>().c_str());
  QStringList list = contextStr.trimmed().split(" ", QString::SkipEmptyParts);
  if (list.size() != 3) {
    qWarning() << "context must be \"key operator operand\". size: " << list.size();
  } else {
    QString key = list[0];

    // Parse operator expression
    QString opStr = list[1];
    Operator op;
    if (opStr == "==") {
      op = Operator::EQUALS;
    } else if (opStr == "!=") {
      op = Operator::NOT_EQUALS;
    } else {
      qWarning("%s is not supported", qPrintable(opStr));
      return nullptr;
    }

    QString value = list[2];

    Context* context = new Context(key, op, value);
    if (context) {
      return context;
    }
  }

  return nullptr;
}

/**
 * @brief YamlUtils::parseMenuNode
 *
 * 'context' decides whether the menu item is shown or not.
 * 'before' decides where the menu item is inserted. In the following case, 'New File' is inserted
 above 'Open...', 'Open...' is inserted above 'Open Recent', 'Save' is inserted below 'Open Recent'
 *
 *  - label: 'New File'
      command: new_file
    - label: 'Open...'
      command: open
      before: 'Open Recent'
    - label: 'Save'
      command: save

 * @param parent
 * @param menuNode
 */
void YamlUtils::parseMenusNode(const std::string& pkgName,
                               QWidget* parent,
                               const YAML::Node& menusNode) {
  if (!menusNode.IsSequence()) {
    qWarning("menusNode must be a sequence.");
    return;
  }

  // Reverse node order to handle 'before' correctly.
  // If before is omitted, before becomes the previous menu item's id automatically
  std::stack<YAML::Node> nodes;
  for (auto it = menusNode.begin(); it != menusNode.end(); it++) {
    nodes.push(*it);
  }

  QString prevId;

  while (!nodes.empty()) {
    YAML::Node node = nodes.top();
    nodes.pop();
    if (!node.IsMap()) {
      qWarning("menu item must be a map");
      continue;
    }

    YAML::Node labelNode = node["label"];
    YAML::Node idNode = node["id"];
    YAML::Node typeNode = node["type"];
    YAML::Node beforeNode = node["before"];
    QString beforeId = beforeNode.IsDefined() && beforeNode.IsScalar()
                           ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                           : prevId;
    QString defaultLabel = labelNode.IsDefined() && labelNode.IsScalar()
                               ? QString::fromUtf8(labelNode.as<std::string>().c_str())
                               : "";
    QString id = idNode.IsDefined() && idNode.IsScalar()
                     ? QString::fromUtf8(idNode.as<std::string>().c_str())
                     : "";
    QString label = defaultLabel;
    if (idNode.IsDefined() && idNode.IsScalar()) {
      label = PluginManager::singleton().translate(
          pkgName + ":menu." + idNode.as<std::string>() + ".label", defaultLabel);
    }
    YAML::Node commandNode = node["command"];
    YAML::Node submenusNode = node["menus"];
    if (submenusNode.IsDefined() && !label.isEmpty() && !id.isEmpty()) {
      QMenu* currentMenu = nullptr;
      if (QAction* action = findAction(parent->actions(), id)) {
        currentMenu = action->menu();
      } else {
        currentMenu = new QMenu(label, parent);
        if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
          QAction* beforeAction =
              beforeId.isEmpty() ? nullptr : findAction(menuBar->actions(), beforeId);
          if (beforeAction) {
            menuBar->insertMenu(beforeAction, currentMenu);
          } else {
            menuBar->addMenu(currentMenu);
          }
        } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
          QAction* beforeAction =
              beforeId.isEmpty() ? nullptr : findAction(parentMenu->actions(), beforeId);
          if (beforeAction) {
            parentMenu->insertMenu(beforeAction, currentMenu);
          } else {
            parentMenu->addMenu(currentMenu);
          }
        }
      }
      parseMenusNode(pkgName, currentMenu, submenusNode);
      prevId = id;
    } else {
      // Check context
      YAML::Node contextNode = node["context"];
      if (contextNode.IsDefined()) {
        Context* context = parseContext(contextNode);
        if (!context || !context->isSatisfied()) {
          continue;
        }
      }

      QAction* action = nullptr;
      if (commandNode.IsDefined() && !label.isEmpty()) {
        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        action = new CommandAction(id, label, command);
      } else if (typeNode.IsDefined() && typeNode.IsScalar()) {
        QString type = QString::fromUtf8(typeNode.as<std::string>().c_str());
        if (type == "separator") {
          action = new QAction(QUuid::createUuid().toString(), parent);
          action->setObjectName(action->text());
          action->setSeparator(true);
        } else {
          qWarning("%s is not supported", qPrintable(type));
        }
      }

      if (!action) {
        continue;
      }

      if (!id.isEmpty() && findAction(parent->actions(), id)) {
        qWarning("%s already exists", qPrintable(id));
        continue;
      }

      if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
        QAction* beforeAction =
            beforeId.isEmpty() ? nullptr : findAction(menuBar->actions(), beforeId);
        if (beforeAction) {
          menuBar->insertAction(beforeAction, action);
        } else {
          menuBar->addAction(action);
        }
      } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
        QAction* beforeAction =
            beforeId.isEmpty() ? nullptr : findAction(parentMenu->actions(), beforeId);
        if (beforeAction) {
          parentMenu->insertAction(beforeAction, action);
        } else {
          parentMenu->addAction(action);
        }
      }
      prevId = id.isEmpty() ? action->objectName() : id;
    }
  }
}

void YamlUtils::parseToolbarsNode(const std::string& pkgName,
                                  const std::string& ymlPath,
                                  QWidget* parent,
                                  const YAML::Node& toolbarsNode) {
  if (!toolbarsNode.IsSequence()) {
    qWarning("toolbarsNode must be a sequence.");
    return;
  }

  // Reverse node order to handle 'before' correctly.
  // If before is omitted, before becomes the previous menu item's id automatically
  std::stack<YAML::Node> nodes;
  for (auto it = toolbarsNode.begin(); it != toolbarsNode.end(); it++) {
    nodes.push(*it);
  }

  QString prevId;

  while (!nodes.empty()) {
    YAML::Node node = nodes.top();
    nodes.pop();
    if (!node.IsMap()) {
      qWarning("toolbar item must be a map");
      continue;
    }

    YAML::Node iconNode = node["icon"];
    YAML::Node labelNode = node["label"];
    YAML::Node commandNode = node["command"];
    YAML::Node tooltipNode = node["tooltip"];
    YAML::Node idNode = node["id"];
    YAML::Node typeNode = node["type"];
    YAML::Node beforeNode = node["before"];
    QString beforeId = beforeNode.IsDefined() && beforeNode.IsScalar()
                           ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                           : prevId;
    QString defaultLabel = labelNode.IsDefined() && labelNode.IsScalar()
                               ? QString::fromUtf8(labelNode.as<std::string>().c_str())
                               : "";
    QString id = idNode.IsDefined() && idNode.IsScalar()
                     ? QString::fromUtf8(idNode.as<std::string>().c_str())
                     : "";
    QString label = defaultLabel;
    if (idNode.IsDefined() && idNode.IsScalar()) {
      label = PluginManager::singleton().translate(
          pkgName + ":toolbar." + idNode.as<std::string>() + ".label", defaultLabel);
    }
    YAML::Node itemsNode = node["items"];
    Window* window = qobject_cast<Window*>(parent);
    // if items is defined, parent must be Window
    if (itemsNode.IsDefined() && !label.isEmpty() && !id.isEmpty() && window) {
      QToolBar* currentToolbar = nullptr;
      if (auto toolbar = window->findToolbar(id)) {
        currentToolbar = toolbar;
      } else {
        currentToolbar = new ToolBar(id, label, parent);
        auto beforeToolbar = beforeId.isEmpty() ? nullptr : window->findToolbar(beforeId);
        if (beforeToolbar) {
          window->insertToolBar(beforeToolbar, currentToolbar);
        } else {
          window->addToolBar(currentToolbar);
        }
      }
      Q_ASSERT(currentToolbar);
      parseToolbarsNode(pkgName, ymlPath, currentToolbar, itemsNode);
      prevId = id;
    } else {
      // Check context
      YAML::Node contextNode = node["context"];
      if (contextNode.IsDefined()) {
        Context* context = parseContext(contextNode);
        if (!context || !context->isSatisfied()) {
          continue;
        }
      }

      QAction* action = nullptr;
      if (commandNode.IsDefined() && iconNode.IsDefined()) {
        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        QString iconPath = QString::fromUtf8(iconNode.as<std::string>().c_str());
        if (!iconPath.startsWith('/')) {
          iconPath = QFileInfo(QString::fromUtf8(ymlPath.c_str())).dir().absoluteFilePath(iconPath);
        }
        action = new CommandAction(id, command, QIcon(iconPath));
        if (tooltipNode.IsDefined()) {
          QString tooltip = QString::fromUtf8(tooltipNode.as<std::string>().c_str());
          tooltip = PluginManager::singleton().translate(
              pkgName + ":toolbar." + idNode.as<std::string>() + ".tooltip", tooltip);
          action->setToolTip(tooltip);
        }
      } else if (typeNode.IsDefined() && typeNode.IsScalar()) {
        QString type = QString::fromUtf8(typeNode.as<std::string>().c_str());
        if (type == "separator") {
          action = new QAction(QUuid::createUuid().toString(), parent);
          action->setObjectName(action->text());
          action->setSeparator(true);
        } else {
          qWarning("%s is not supported", qPrintable(type));
        }
      }

      QToolBar* toolbar = qobject_cast<QToolBar*>(parent);
      if (!action || !toolbar) {
        continue;
      }

      if (!id.isEmpty() && findAction(toolbar->actions(), id)) {
        qWarning("%s already exists", qPrintable(id));
        continue;
      }

      QAction* beforeAction =
          beforeId.isEmpty() ? nullptr : findAction(toolbar->actions(), beforeId);
      if (beforeAction) {
        toolbar->insertAction(beforeAction, action);
      } else {
        toolbar->addAction(action);
      }
      prevId = id.isEmpty() ? action->objectName() : id;
    }
  }
}
