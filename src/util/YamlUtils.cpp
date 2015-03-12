#include <stack>
#include <QStringList>
#include <QDebug>
#include <QMenuBar>
#include <QUuid>

#include "YamlUtils.h"
#include "Context.h"
#include "CommandAction.h"

namespace {
QAction* findAction(QList<QAction*> actions, const QString& label) {
  foreach (QAction* action, actions) {
    if (action->text().replace("&", "") == label) {
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
void YamlUtils::parseMenuNode(QWidget* parent, YAML::Node menuNode) {
  if (!menuNode.IsSequence()) {
    qWarning("menuNode must be a sequence.");
    return;
  }

  // Reverse node order to handle 'before' correctly
  std::stack<YAML::Node> nodes;
  for (auto it = menuNode.begin(); it != menuNode.end(); it++) {
    nodes.push(*it);
  }

  QString prevLabel;

  while (!nodes.empty()) {
    YAML::Node node = nodes.top();
    nodes.pop();
    if (!node.IsMap()) {
      qWarning("menu item must be a map");
      continue;
    }

    YAML::Node labelNode = node["label"];
    YAML::Node typeNode = node["type"];
    YAML::Node beforeNode = node["before"];
    QString before = beforeNode.IsDefined() && beforeNode.IsScalar()
                         ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                         : prevLabel;
    QString label = labelNode.IsDefined() && labelNode.IsScalar()
                        ? QString::fromUtf8(labelNode.as<std::string>().c_str())
                        : "";
    YAML::Node commandNode = node["command"];
    YAML::Node submenuNode = node["submenu"];
    if (submenuNode.IsDefined() && !label.isEmpty()) {
      QMenu* currentMenu;
      if (QAction* action = findAction(parent->actions(), label)) {
        currentMenu = action->menu();
      } else {
        currentMenu = new QMenu(label, parent);
        if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
          QAction* beforeAction =
              before.isEmpty() ? nullptr : findAction(menuBar->actions(), before);
          if (beforeAction) {
            menuBar->insertMenu(beforeAction, currentMenu);
          } else {
            menuBar->addMenu(currentMenu);
          }
        } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
          QAction* beforeAction =
              before.isEmpty() ? nullptr : findAction(parentMenu->actions(), before);
          if (beforeAction) {
            parentMenu->insertMenu(beforeAction, currentMenu);
          } else {
            parentMenu->addMenu(currentMenu);
          }
        }
      }
      parseMenuNode(currentMenu, submenuNode);
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
        action = new CommandAction(label, command);
      } else if (typeNode.IsDefined() && typeNode.IsScalar()) {
        QString type = QString::fromUtf8(typeNode.as<std::string>().c_str());
        if (type == "separator") {
          action = new QAction(QUuid::createUuid().toString(), parent);
          action->setSeparator(true);
        } else {
          qWarning("%s is not supported", qPrintable(type));
        }
      }

      if (!action) {
        continue;
      }

      if (!label.isEmpty() && findAction(parent->actions(), label)) {
        qWarning("%s already exists", qPrintable(label));
        continue;
      }

      if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
        QAction* beforeAction = before.isEmpty() ? nullptr : findAction(menuBar->actions(), before);
        if (beforeAction) {
          menuBar->insertAction(beforeAction, action);
        } else {
          menuBar->addAction(action);
        }
      } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
        QAction* beforeAction =
            before.isEmpty() ? nullptr : findAction(parentMenu->actions(), before);
        if (beforeAction) {
          parentMenu->insertAction(beforeAction, action);
        } else {
          parentMenu->addAction(action);
        }
      }
      prevLabel = label.isEmpty() ? action->text() : label;
    }
  }
}
