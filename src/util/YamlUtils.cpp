#include <stack>
#include <QStringList>
#include <QDebug>
#include <QMenuBar>

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
 * 'before' decides where the menu item is inserted. In the following case, 'New File' is inserted above 'Open...', 'Open...' is inserted above 'Open Recent', 'Save' is inserted below 'Open Recent'
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
    if (labelNode.IsDefined() && labelNode.IsScalar()) {
      QString label = QString::fromUtf8(labelNode.as<std::string>().c_str());
      YAML::Node commandNode = node["command"];
      YAML::Node submenuNode = node["submenu"];
      YAML::Node beforeNode = node["before"];
      QString before = beforeNode.IsDefined()
                           ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                           : prevLabel;
      if (submenuNode.IsDefined()) {
        QMenu* currentMenu;
        if (QAction* action = findAction(parent->actions(), label)) {
          currentMenu = action->menu();
        } else {
          currentMenu = new QMenu(label, parent);
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            QAction* beforeAction = before.isEmpty() ? nullptr : findAction(menuBar->actions(), before);
            if (beforeAction) {
              menuBar->insertMenu(beforeAction, currentMenu);
            } else {
              menuBar->addMenu(currentMenu);
            }
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            QAction* beforeAction = before.isEmpty() ? nullptr : findAction(parentMenu->actions(), before);
            if (beforeAction) {
              parentMenu->insertMenu(beforeAction, currentMenu);
            } else {
              parentMenu->addMenu(currentMenu);
            }
          }
        }
        parseMenuNode(currentMenu, submenuNode);
      } else if (commandNode.IsDefined()) {
        // Check context
        YAML::Node contextNode = node["context"];
        if (contextNode.IsDefined()) {
          Context* context = parseContext(contextNode);
          if (!context || !context->isSatisfied()) {
            continue;
          }
        }

        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        auto commandAction = new CommandAction(label, command);
        if (!findAction(parent->actions(), label)) {
          if (QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent)) {
            QAction* beforeAction = before.isEmpty() ? nullptr : findAction(menuBar->actions(), before);
            if (beforeAction) {
              menuBar->insertAction(beforeAction, commandAction);
            } else {
              menuBar->addAction(commandAction);
            }
          } else if (QMenu* parentMenu = qobject_cast<QMenu*>(parent)) {
            QAction* beforeAction = before.isEmpty() ? nullptr : findAction(parentMenu->actions(), before);
            if (beforeAction) {
              parentMenu->insertAction(beforeAction, commandAction);
            } else {
              parentMenu->addAction(commandAction);
            }
          }
        } else {
          qWarning("%s already exists", qPrintable(label));
        }
      }

      prevLabel = label;
    }
  }
}
