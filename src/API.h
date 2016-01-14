#pragma once

#include <boost/optional.hpp>
#include <functional>
#include <unordered_map>
#include <string>
#include <QObject>

#include "core/macros.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"

class TextEditView;
class TabView;
class TabViewGroup;
class Window;

class API : public QObject, public core::Singleton<API> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(API)

 public:
  ~API() = default;

public slots:
  void setFont(const QString& family, int size);
  void hideActiveFindReplacePanel();
  QList<Window*> windows();
  boost::optional<QString> getConfig(const QString& name);
  QString version();

 private:
  friend class core::Singleton<API>;
  API() = default;
};
