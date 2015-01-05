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
  std::unique_ptr<Settings> settings;
  QFont::Weight fontWeight;
  bool isItalic;
  bool isUnderline;

  ScopeSetting()
      : settings(nullptr), fontWeight(QFont::Normal), isItalic(false), isUnderline(false) {}
};

class Theme {
  DISABLE_COPY(Theme)

 public:
  Theme() : gutterSettings(nullptr), scopeSettings(QVector<ScopeSetting*>(0)) {}
  ~Theme() = default;
  DEFAULT_MOVE(Theme)

  static Theme* loadTheme(const QString& filename);
  std::unique_ptr<QTextCharFormat> getFormat(const QString& scope);

  std::unique_ptr<Settings> gutterSettings;
  QFont::Weight gutterFontWeight;
  bool isGutterItalic;
  bool isGutterUnderline;
  QString name;
  QVector<ScopeSetting*> scopeSettings;
  QUuid uuid;

 private:
  ScopeSetting* closestMatchingSetting(const QString& scope);
};
