#pragma once

#include <boost/optional.hpp>
#include <msgpack/rpc/protocol.h>
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
class InputDialog;

class API : public QObject, public core::Singleton<API> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(API)

 public:
  ~API() = default;

  // notify functions
  Q_INVOKABLE void alert(const QString& msg);
  Q_INVOKABLE void loadKeymap(const QString& pkgName, const QString& ymlPath);
  Q_INVOKABLE void loadMenu(const QString& pkgName, const QString& ymlPath);
  Q_INVOKABLE void loadToolbar(const QString& pkgName, const QString& ymlPath);
  Q_INVOKABLE void loadConfig(const QString& pkgName, const QString& ymlPath);
  Q_INVOKABLE void registerCommands(QVariantList commands);
  Q_INVOKABLE void unregisterCommands(QList<QString> commands);
  Q_INVOKABLE void registerCondition(const QString& condition);
  Q_INVOKABLE void unregisterCondition(const QString& condition);
  Q_INVOKABLE void open(const QString& pathStr);
  Q_INVOKABLE void dispatchCommand(const QString& typeStr,
                                   const QString& key,
                                   bool autorep,
                                   bool altKey,
                                   bool ctrlKey,
                                   bool metaKey,
                                   bool shiftKey);
  Q_INVOKABLE void setFont(const QString& family, int size);
  Q_INVOKABLE void hideActiveFindReplacePanel();

  // request functions
  Q_INVOKABLE TextEditView* activeTextEditView();
  Q_INVOKABLE TabView* activeTabView();
  Q_INVOKABLE TabViewGroup* activeTabViewGroup();
  Q_INVOKABLE Window* activeWindow();
  Q_INVOKABLE QStringList showFileAndFolderDialog(const QString& caption);
  Q_INVOKABLE QStringList showFilesDialog(const QString& caption);
  Q_INVOKABLE boost::optional<QString> showFolderDialog(const QString& caption);
  Q_INVOKABLE QList<Window*> windows();
  Q_INVOKABLE boost::optional<QString> getConfig(const QString& name);
  Q_INVOKABLE QString version();
  Q_INVOKABLE InputDialog* newInputDialog();

 private:
  friend class core::Singleton<API>;
  API();

  QStringList showDialogImpl(const QString& caption, DialogUtils::MODE mode);
};

Q_DECLARE_METATYPE(boost::optional<QString>)
