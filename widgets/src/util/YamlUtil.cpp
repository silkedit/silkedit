#include <stack>
#include <QStringList>
#include <QDebug>
#include <QMenuBar>
#include <QUuid>
#include <QToolbar>
#include <QDir>
#include <QRegularExpression>

#include "YamlUtil.h"
#include "CommandAction.h"
#include "Window.h"
#include "core/ConditionExpression.h"
#include "core/PackageToolBar.h"
#include "core/PackageMenu.h"
#include "core/PackageAction.h"
#include "core/Regexp.h"
#include "core/JSValue.h"
#include "core/Util.h"
#include "core/Config.h"
#include "core/V8Util.h"

using core::Condition;
using core::PackageMenu;
using core::PackageAction;
using core::PackageToolBar;
using core::ConfigDefinition;
using core::ConditionExpression;
using core::AndConditionExpression;
using core::Regexp;
using core::JSNull;
using core::Util;
using core::Config;
using core::V8Util;

namespace {

QString getAbsolutePath(const QString& ymlPath, const QString& path) {
  QString targetPath = path;
  if (!path.startsWith('/')) {
    targetPath = QFileInfo(ymlPath).dir().absoluteFilePath(path);
  }
  return targetPath;
}

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

QMap<QString, YAML::Node> YamlUtil::s_nodeCache;

boost::optional<AndConditionExpression> YamlUtil::parseCondition(const YAML::Node& conditionNode) {
  QString conditionStr = QString::fromUtf8(conditionNode.as<std::string>().c_str());
  QStringList strList = conditionStr.trimmed().split("&&", QString::SkipEmptyParts);
  QSet<ConditionExpression> conSet;
  for (const auto& str : strList) {
    if (auto cond = parseValueCondition(str)) {
      conSet.insert(*cond);
    }
  }

  if (conSet.isEmpty()) {
    return boost::none;
  } else {
    return AndConditionExpression(conSet);
  }
}

boost::optional<ConditionExpression> YamlUtil::parseValueCondition(const QString& str) {
  QStringList list = str.trimmed().split(" ", QString::SkipEmptyParts);

  QString key;
  QString op;
  QVariant value;
  if (list.size() == 1) {
    key = list[0];
    op = Condition::equalsOperator;
    value = QVariant::fromValue(true);
  } else if (list.size() == 3) {
    key = list[0];
    op = list[1];
    value = Util::toVariant(list[2]);
  } else {
    qWarning() << "condition must be \"key operator operand\". size: " << list.size();
    return boost::none;
  }

  return ConditionExpression(key, op, value);
}

/**
 * @brief YamlUtil::parseMenuNode
 *
 * 'condition' decides whether the menu item is shown or not.
 * 'before' decides where the menu item is inserted. In the following case, 'New File' is inserted
 above 'Open...', 'Open...' is inserted above 'Open Recent', 'Save' is inserted below 'Open Recent'
 *
 *  - title: 'New File'
      command: new_file
    - title: 'Open...'
      command: open
      before: 'Open Recent'
    - title: 'Save'
      command: save

 * @param parent
 * @param menuNode
 */
void YamlUtil::parseMenuNode(const QString& pkgName,
                             const QString& pkgPath,
                             QWidget* parent,
                             const YAML::Node& menuNode) {
  if (!menuNode.IsSequence()) {
    qWarning("menuNode must be a sequence.");
    return;
  }

  // Reverse node order to handle 'before' correctly.
  // If before is omitted, before becomes the previous menu item's id automatically
  std::stack<YAML::Node> nodes;
  for (auto it = menuNode.begin(); it != menuNode.end(); it++) {
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

    YAML::Node titleNode = node["title"];
    YAML::Node idNode = node["id"];
    YAML::Node typeNode = node["type"];
    YAML::Node beforeNode = node["before"];
    QString beforeId = beforeNode.IsDefined() && beforeNode.IsScalar()
                           ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                           : prevId;
    QString defaultTitle = titleNode.IsDefined() && titleNode.IsScalar()
                               ? QString::fromUtf8(titleNode.as<std::string>().c_str())
                               : "";
    QString id = idNode.IsDefined() && idNode.IsScalar()
                     ? QString::fromUtf8(idNode.as<std::string>().c_str())
                     : "";
    QString title = defaultTitle;
    if (idNode.IsDefined() && idNode.IsScalar()) {
      title = translate(pkgPath, QStringLiteral("menu.%2.title")
                                     .arg(QString::fromUtf8(idNode.as<std::string>().c_str())),
                        defaultTitle);
    }
    YAML::Node commandNode = node["command"];
    YAML::Node submenuNode = node["menu"];
    if (submenuNode.IsDefined() && !id.isEmpty()) {
      QMenu* currentMenu = nullptr;
      if (QAction* action = findAction(parent->actions(), id)) {
        currentMenu = action->menu();
      } else if (!title.isEmpty()) {
        currentMenu = new PackageMenu(title, pkgName, parent);
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
      } else {
        qWarning() << "id is invalid and title is empty";
        continue;
      }

      Q_ASSERT(currentMenu);
      parseMenuNode(pkgName, pkgPath, currentMenu, submenuNode);
      prevId = id;
    } else {
      // Check if condition
      YAML::Node ifNode = node["if"];
      boost::optional<AndConditionExpression> condition;
      if (ifNode.IsDefined()) {
        condition = parseCondition(ifNode);
      }

      QAction* action = nullptr;
      if (!title.isEmpty()) {
        QString command;
        if (commandNode.IsDefined()) {
          command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        }
        action =
            new CommandAction(id, title, command, nullptr, CommandArgument(), condition, pkgName);
      } else if (typeNode.IsDefined() && typeNode.IsScalar()) {
        const QString& type = QString::fromUtf8(typeNode.as<std::string>().c_str());
        if (type == "separator") {
          action = new PackageAction(QUuid::createUuid().toString(), pkgName, parent, condition);
          action->setObjectName(action->text());
          action->setSeparator(true);
        } else {
          qWarning("%s is not supported", qPrintable(type));
        }
      }

      if (!action) {
        continue;
      }

      if (!id.isEmpty()) {
        auto action = findAction(parent->actions(), id);
        if (action) {
          parent->removeAction(action);
        }
      }

      // check checkable
      YAML::Node checkableNode = node["checkable"];
      if (checkableNode.IsDefined() && checkableNode.IsScalar() && checkableNode.as<bool>()) {
        Q_ASSERT(action);
        action->setCheckable(true);
        if (Config::singleton().contains(id)) {
          action->setChecked(Config::singleton().get(id, false));
        } else {
          auto defaultValue = Config::singleton().defaultValue(id);
          if (defaultValue.canConvert<bool>()) {
            action->setChecked(Config::singleton().defaultValue(id).toBool());
          }
        }
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

void YamlUtil::parseToolbarNode(const QString& pkgName,
                                const QString& ymlPath,
                                QWidget* parent,
                                const YAML::Node& toolbarNode) {
  if (!toolbarNode.IsSequence()) {
    qWarning("toolbarNode must be a sequence.");
    return;
  }
  const auto& pkgPath = ymlPath.left(ymlPath.lastIndexOf('/'));
  // Reverse node order to handle 'before' correctly.
  // If before is omitted, before becomes the previous menu item's id automatically
  std::stack<YAML::Node> nodes;
  for (auto it = toolbarNode.begin(); it != toolbarNode.end(); it++) {
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
    YAML::Node titleNode = node["title"];
    YAML::Node commandNode = node["command"];
    YAML::Node tooltipNode = node["tooltip"];
    YAML::Node idNode = node["id"];
    YAML::Node typeNode = node["type"];
    YAML::Node beforeNode = node["before"];
    QString beforeId = beforeNode.IsDefined() && beforeNode.IsScalar()
                           ? QString::fromUtf8(beforeNode.as<std::string>().c_str())
                           : prevId;
    QString defaultTitle = titleNode.IsDefined() && titleNode.IsScalar()
                               ? QString::fromUtf8(titleNode.as<std::string>().c_str())
                               : "";
    QString id = idNode.IsDefined() && idNode.IsScalar()
                     ? QString::fromUtf8(idNode.as<std::string>().c_str())
                     : "";
    QString title = defaultTitle;
    if (idNode.IsDefined() && idNode.IsScalar()) {
      title = translate(pkgPath, QString("toolbar.%2.title")
                                     .arg(QString::fromUtf8(idNode.as<std::string>().c_str())),
                        defaultTitle);
    }
    YAML::Node itemsNode = node["items"];
    Window* window = qobject_cast<Window*>(parent);
    // if items is defined, parent must be Window
    if (itemsNode.IsDefined() && !title.isEmpty() && !id.isEmpty() && window) {
      QToolBar* currentToolbar = nullptr;
      if (auto toolbar = window->findToolbar(id)) {
        currentToolbar = toolbar;
      } else {
        currentToolbar = new PackageToolBar(id, title, parent, pkgName);
        auto beforeToolbar = beforeId.isEmpty() ? nullptr : window->findToolbar(beforeId);
        if (beforeToolbar) {
          window->insertToolBar(beforeToolbar, currentToolbar);
        } else {
          window->addToolBar(currentToolbar);
        }
      }
      Q_ASSERT(currentToolbar);
      parseToolbarNode(pkgName, ymlPath, currentToolbar, itemsNode);
      prevId = id;
    } else {
      // Check if condition
      YAML::Node ifNode = node["if"];
      boost::optional<AndConditionExpression> condition;
      if (ifNode.IsDefined()) {
        condition = parseCondition(ifNode);
      }

      QAction* action = nullptr;
      if (commandNode.IsDefined() && iconNode.IsDefined()) {
        QString command = QString::fromUtf8(commandNode.as<std::string>().c_str());
        if (iconNode.Type() == YAML::NodeType::Scalar) {
          QString iconPath =
              getAbsolutePath(ymlPath, QString::fromUtf8(iconNode.as<std::string>().c_str()));
          if (QFileInfo::exists(iconPath)) {
            action = new CommandAction(id, command, QIcon(iconPath), nullptr, CommandArgument(),
                                       condition, pkgName);
          } else {
            qWarning() << iconPath << "doesn't exist";
          }
        } else if (iconNode.Type() == YAML::NodeType::Map) {
          QMap<QString, QString> icons;
          for (auto it = iconNode.begin(); it != iconNode.end(); ++it) {
            YAML::Node key = it->first;
            YAML::Node value = it->second;
            const auto& iconPath =
                getAbsolutePath(ymlPath, QString::fromUtf8(value.as<std::string>().c_str()));
            if (QFileInfo::exists(iconPath)) {
              icons.insert(QString::fromUtf8(key.as<std::string>().c_str()), iconPath);
            } else {
              qWarning() << iconPath << "doesn't exist";
            }
          }
          action =
              new CommandAction(id, command, icons, nullptr, CommandArgument(), condition, pkgName);
        }
        if (action && tooltipNode.IsDefined()) {
          QString tooltip = QString::fromUtf8(tooltipNode.as<std::string>().c_str());
          tooltip =
              translate(pkgPath, QString("toolbar.%2.tooltip")
                                     .arg(QString::fromUtf8(idNode.as<std::string>().c_str())),
                        tooltip);
          action->setToolTip(tooltip);
        }
      } else if (typeNode.IsDefined() && typeNode.IsScalar()) {
        QString type = QString::fromUtf8(typeNode.as<std::string>().c_str());
        if (type == "separator") {
          action = new PackageAction(QUuid::createUuid().toString(), pkgName, parent, condition);
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

      if (!id.isEmpty()) {
        auto action = findAction(toolbar->actions(), id);
        if (action) {
          toolbar->removeAction(action);
        }
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

QList<ConfigDefinition> YamlUtil::parseConfig(const QString& pkgName, const QString& ymlPath) {
  QList<ConfigDefinition> defs;

  const QString& pkgPath = ymlPath.left(ymlPath.lastIndexOf('/'));

  try {
    YAML::Node rootNode = YAML::LoadFile(ymlPath.toUtf8().constData());
    if (!rootNode.IsMap()) {
      qWarning("root node must be a map");
      return QList<ConfigDefinition>();
    }

    YAML::Node configNode = rootNode["config"];
    if (!configNode.IsMap()) {
      qWarning("config node must be a map");
      return QList<ConfigDefinition>();
    }

    for (auto configIter = configNode.begin(); configIter != configNode.end(); ++configIter) {
      if (configIter->first.IsScalar() && configIter->second.IsMap()) {
        QString configName = QString::fromUtf8(configIter->first.as<std::string>().c_str());
        YAML::Node defNode = configIter->second;
        if (!defNode.IsMap()) {
          qWarning("%s node must be a map", qPrintable(configName));
          return QList<ConfigDefinition>();
        }

        QString title = QString::fromUtf8(defNode["title"].as<std::string>().c_str());
        title = translate(pkgPath, QString("config.%2.title").arg(configName), title);
        QString description;
        // description is optional
        if (defNode["description"].IsScalar()) {
          description = QString::fromUtf8(defNode["description"].as<std::string>().c_str());
          description =
              translate(pkgPath, QString("config.%2.description").arg(configName), description);
        }
        QString type = QString::fromUtf8(defNode["type"].as<std::string>().c_str());
        QVariant defaultValue;

        // default is optional
        YAML::Node defaultNode = defNode["default"];
        if (type == "string") {
          if (defaultNode.IsScalar()) {
            defaultValue = QVariant(QString::fromUtf8(defaultNode.as<std::string>().c_str()));
          } else {
            defaultValue = QVariant("");
          }
        } else if (type == "integer") {
          if (defaultNode.IsScalar()) {
            defaultValue = QVariant(defaultNode.as<int>());
          } else {
            defaultValue = QVariant(0);
          }
        } else if (type == "number") {
          if (defaultNode.IsScalar()) {
            defaultValue = QVariant(defaultNode.as<double>());
          } else {
            defaultValue = QVariant(0.0);
          }
        } else if (type == "boolean") {
          if (defaultNode.IsScalar()) {
            defaultValue = QVariant(defaultNode.as<bool>());
          } else {
            defaultValue = QVariant(false);
          }
        } else {
          qWarning("invalid tyep: %s", qPrintable(type));
          continue;
        }
        Q_ASSERT(defaultValue.isValid());
        defs.append(ConfigDefinition{pkgName + "." + configName, title, description, defaultValue});
      }
    }
  } catch (const std::runtime_error& ex) {
    qWarning("Unable to load %s. Cause: %s", qPrintable(ymlPath), ex.what());
  }

  return defs;
}

void YamlUtil::translate(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& pkgPath =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  const auto& key =
      V8Util::toQString(args[1]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  const auto& defaultValue =
      V8Util::toQString(args[2]->ToString(isolate->GetCurrentContext()).ToLocalChecked());

  args.GetReturnValue().Set(
      V8Util::toV8Value(isolate, QVariant(translate(pkgPath, key, defaultValue))));
}

void YamlUtil::clearTranslationCache(const v8::FunctionCallbackInfo<v8::Value>&) {
  s_nodeCache.clear();
}

QString YamlUtil::translate(const QString& pkgPath,
                            const QString& key,
                            const QString& defaultValue) {
  return translate(pkgPath, Config::singleton().locale(), key, defaultValue);
}

QString YamlUtil::translate(const QString& pkgPath,
                            const QString& locale,
                            const QString& key,
                            const QString& defaultValue) {
  QStringList translationPaths;
  const QString& path = QDir::cleanPath(pkgPath + QStringLiteral("/locales/") + locale +
                                        QStringLiteral("/translation.yml"));
  if (QFileInfo::exists(path)) {
    translationPaths.append(path);
  }

  auto indexOf_ = locale.indexOf('_');
  if (indexOf_ > 0) {
    const QString& path =
        QDir::cleanPath(pkgPath + QStringLiteral("/locales/") + locale.left(indexOf_) +
                        QStringLiteral("/translation.yml"));
    if (QFileInfo::exists(path)) {
      translationPaths.append(path);
    }
  }

  for (const auto& translationPath : translationPaths) {
    try {
      YAML::Node rootNode;
      if (s_nodeCache.contains(path)) {
        rootNode = s_nodeCache[path];
      } else {
        rootNode = YAML::LoadFile(translationPath.toUtf8().constData());
        if (!rootNode.IsMap()) {
          qWarning() << "root node must be a map";
          return defaultValue;
        }
        s_nodeCache.insert(path, rootNode);
      }

      YAML::Node valueNode = rootNode[key.toUtf8().constData()];

      if (valueNode) {
        return QString::fromUtf8(valueNode.as<std::string>().c_str());
      }
    } catch (const std::runtime_error& ex) {
      qWarning() << "Unable to load" << translationPath << "Cause:" << ex.what();
    }
  }

  return defaultValue;
}
