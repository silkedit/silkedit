#pragma once

#include <node.h>
#include <boost/optional.hpp>
#include <functional>
#include <unordered_map>
#include <string>
#include <QObject>

#include "core/macros.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"
#include "util/DialogUtils.h"

class TextEditView;
class TabView;
class TabViewGroup;
class Window;

class NODE_EXTERN API : public QObject, public core::Singleton<API> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(API)

 public:
  ~API() = default;

public slots:
  void alert(const QString& msg);
  void loadKeymap(const QString& pkgName, const QString& ymlPath);
  void loadMenu(const QString& pkgName, const QString& ymlPath);
  void loadToolbar(const QString& pkgName, const QString& ymlPath);
  void loadConfig(const QString& pkgName, const QString& ymlPath);
  void registerCommands(QVariantList commands);
  void unregisterCommands(QVariantList commands);
  void registerCondition(const QString& condition);
  void unregisterCondition(const QString& condition);
  void open(const QString& pathStr);
  void setFont(const QString& family, int size);
  void hideActiveFindReplacePanel();
  TextEditView* activeTextEditView();
  TabView* activeTabView();
  TabViewGroup* activeTabViewGroup();
  Window* activeWindow();
  QStringList showFileAndFolderDialog(const QString& caption);
  QStringList showFilesDialog(const QString& caption);
  boost::optional<QString> showFolderDialog(const QString& caption);
  QList<Window*> windows();
  boost::optional<QString> getConfig(const QString& name);
  QString version();

 private:
  friend class core::Singleton<API>;
  API() = default;

  QStringList showDialogImpl(const QString& caption, DialogUtils::MODE mode);
};
