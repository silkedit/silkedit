#pragma once

#include <memory>
#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QUuid>
#include <QTextCharFormat>

#include "macros.h"
#include "LanguageParser.h"

typedef QMap<QString, QColor> Settings;

struct ScopeSetting {
  QString name;
  QStringList scopes;
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
  std::unique_ptr<QTextCharFormat> spice(const QString& scope);

  Settings* gutterSettings;
  QString name;
  QVector<ScopeSetting*> settings;
  QUuid uuid;

 private:
  ScopeSetting* closestMatchingSetting(const QString& scope);
};
