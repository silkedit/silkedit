#pragma once

#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QUuid>

#include "macros.h"

typedef QMap<QString, QColor> Settings;

struct ScopeSetting {
  QString name;
  QString scope;
  Settings* settings;

  ScopeSetting():settings(nullptr){}
};

class Theme {
  DISABLE_COPY(Theme)

 public:
  Theme(): gutterSettings(nullptr), settings(QVector<ScopeSetting*>(0)){}
  ~Theme() = default;
  DEFAULT_MOVE(Theme)

  static Theme* loadTheme(const QString& filename);

  Settings* gutterSettings;
  QString name;
  QVector<ScopeSetting*> settings;
  QUuid uuid;

 private:
};
