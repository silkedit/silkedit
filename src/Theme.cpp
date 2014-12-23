#include "Theme.h"
#include "PListParser.h"

namespace {
const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString uuidStr = "uuid";
const QString foregroundStr = "foreground";
const QString backgroundStr = "background";

void parseSettings(Settings* settings,
                   QFont::Weight* fontWeight,
                   bool* isItalic,
                   bool* isUnderline,
                   QVariant var) {
  if (!var.canConvert<QVariantMap>()) {
    return;
  }

  QVariantMap map = var.toMap();
  QMapIterator<QString, QVariant> iter(map);
  while (iter.hasNext()) {
    iter.next();
    QString key = iter.key();
    if (key == "fontStyle") {
      QStringList styles = iter.value().toString().split(' ');
      foreach(const QString & style, styles) {
        if (style == "bold") {
          *fontWeight = QFont::Bold;
        } else if (style == "italic") {
          *isItalic = true;
        } else if (style == "underline") {
          *isUnderline = true;
        }
      }
    } else {
      QColor color(iter.value().toString());
      if (color.isValid()) {
        (*settings)[key] = color;
      }
    }
  }
}

ScopeSetting* toScopeSetting(QVariant var) {
  if (!var.canConvert<QVariantMap>()) {
    return nullptr;
  }

  QVariantMap map = var.toMap();
  ScopeSetting* scopeSetting = new ScopeSetting();

  // name
  if (map.contains(nameStr)) {
    scopeSetting->name = map.value(nameStr).toString();
  }

  // scope
  if (map.contains(scopeStr)) {
    QStringList scopeList = map.value(scopeStr).toString().split(',');
    for (int i = 0; i < scopeList.length(); i++) {
      scopeList[i] = scopeList[i].trimmed();
    }
    scopeSetting->scopes = scopeList;
  }

  // settings
  if (map.contains(settingsStr)) {
    scopeSetting->settings.reset(new Settings());
    parseSettings(scopeSetting->settings.get(),
                  &(scopeSetting->fontWeight),
                  &(scopeSetting->isItalic),
                  &(scopeSetting->isUnderline),
                  map.value(settingsStr));
  }

  return scopeSetting;
}
}

Theme* Theme::loadTheme(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("unable to open a file");
    return nullptr;
  }

  QVariant root = PListParser::parsePList(&file);
  if (!root.canConvert<QVariantMap>()) {
    qWarning("root is not dict");
    return nullptr;
  }

  Theme* theme = new Theme();
  QVariantMap rootMap = root.toMap();

  // gutterSettings
  const QString gutterSettingsStr = "gutterSettings";
  if (rootMap.contains(gutterSettingsStr)) {
    theme->gutterSettings.reset(new Settings());
    parseSettings(theme->gutterSettings.get(),
                  &(theme->gutterFontWeight),
                  &(theme->isGutterItalic),
                  &(theme->isGutterUnderline),
                  rootMap.value(gutterSettingsStr));
  }

  // name
  if (rootMap.contains(nameStr)) {
    theme->name = rootMap.value(nameStr).toString();
  }

  // settings
  if (rootMap.contains(settingsStr)) {
    QVariantList settingList = rootMap.value(settingsStr).toList();
    foreach(const QVariant & var, settingList) { theme->settings.append(toScopeSetting(var)); }
  }

  // UUID
  if (rootMap.contains(uuidStr)) {
    theme->uuid = rootMap.value(uuidStr).toString();
  }

  return theme;
}

ScopeSetting* Theme::closestMatchingSetting(const QString& scope) {
  QString na = scope;
  while (na.length() > 0) {
    QString sn = na;
    int i = sn.lastIndexOf(" ");
    if (i != -1) {
      sn = sn.right(sn.length() - (i + 1));
    }

    foreach(ScopeSetting * j, settings) {
      if (j->scopes.contains(sn)) {
        return j;
      }
    }

    int i2 = na.lastIndexOf(".");
    if (i2 == -1) {
      break;
    } else if (i > i2) {
      na = na.left(i);
    } else {
      na = na.left(i2).trimmed();
    }
  }
  return settings[0];
}

std::unique_ptr<QTextCharFormat> Theme::spice(const QString& scope) {
  if (settings.isEmpty())
    return nullptr;

  QTextCharFormat* format = new QTextCharFormat();
  ScopeSetting* def = settings[0];
  ScopeSetting* s = closestMatchingSetting(scope);
  if (s) {
    // foreground
    QColor fg = s->settings->value(foregroundStr, def->settings->value(foregroundStr));
    Q_ASSERT(fg.isValid());
    format->setForeground(fg);

    // background
    QColor bg = s->settings->value(backgroundStr, def->settings->value(backgroundStr));
    Q_ASSERT(bg.isValid());
    format->setBackground(bg);

    // font style
    format->setFontWeight(s->fontWeight);
    format->setFontItalic(s->isItalic);
    format->setFontUnderline(s->isUnderline);
  }

  return std::unique_ptr<QTextCharFormat>(format);
}
