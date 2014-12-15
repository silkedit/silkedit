#include "Theme.h"
#include "PListParser.h"

namespace {
const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString uuidStr = "uuid";

Settings* toSettings(QVariant var) {
  if (!var.canConvert<QVariantMap>()) {
    return nullptr;
  }

  Settings* settings = new Settings();
  QVariantMap map = var.toMap();
  QMapIterator<QString, QVariant> iter(map);
  while (iter.hasNext()) {
    iter.next();
    QString key = iter.key();
    QColor color(iter.value().toString());
    if (color.isValid()) {
      (*settings)[key] = color;
    }
  }

  return settings;
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
    scopeSetting->scope = map.value(scopeStr).toString();
  }

  // settings
  if (map.contains(settingsStr)) {
    scopeSetting->settings = toSettings(map.value(settingsStr));
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
    theme->gutterSettings = toSettings(rootMap.value(gutterSettingsStr));
  }

  // name
  if (rootMap.contains(nameStr)) {
    theme->name = rootMap.value(nameStr).toString();
  }

  // settings
  if (rootMap.contains(settingsStr)) {
    QVariantList settingList = rootMap.value(settingsStr).toList();
    foreach (const QVariant& var, settingList) {
      theme->settings.append(toScopeSetting(var));
    }
  }

  // UUID
  if (rootMap.contains(uuidStr)) {
    theme->uuid = rootMap.value(uuidStr).toString();
  }

  return theme;
}
