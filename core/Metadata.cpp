#include <QFile>
#include <QVariant>

#include "Metadata.h"
#include "PListParser.h"
#include "Regexp.h"

namespace {
const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString increaseIndentPatternStr = "increaseIndentPattern";
const QString decreaseIndentPatternStr = "decreaseIndentPattern";
const QString bracketIndentNextLinePatternStr = "bracketIndentNextLinePattern";
const QString disableIndentNextLinePatternStr = "disableIndentNextLinePattern";
const QString unIndentedLinePatternStr = "unIndentedLinePattern";
}

namespace core {

std::unordered_map<QString, std::unique_ptr<Metadata>> Metadata::s_scopeMetadataMap;

Metadata::Metadata(const QString& scope) : m_scope(scope) {
}

void Metadata::setIncreaseIndentPattern(const QString& pattern) {
  m_increaseIndentPattern.reset(Regexp::compile(pattern));
}

void Metadata::setDecreateIndentPattern(const QString& pattern) {
  m_decreaseIndentPattern.reset(Regexp::compile(pattern));
}

void Metadata::setBracketIndentNextLinePattern(const QString& pattern) {
  m_bracketIndentNextLinePattern.reset(Regexp::compile(pattern));
}

void Metadata::setDisableIndentNextLinePattern(const QString& pattern) {
  m_disableIndentNextLinePattern.reset(Regexp::compile(pattern));
}

void Metadata::setUnIndentedLinePattern(const QString& pattern) {
  m_unIndentedLinePattern.reset(Regexp::compile(pattern));
}

Metadata* Metadata::get(const QString& scope) {
  if (s_scopeMetadataMap.count(scope) != 0) {
    return s_scopeMetadataMap[scope].get();
  }
  return nullptr;
}

void Metadata::load(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("unable to open a file");
    return;
  }

  QVariant root = PListParser::parsePList(&file);
  if (!root.canConvert<QVariantMap>()) {
    qWarning("root is not dict");
    return;
  }

  QVariantMap rootMap = root.toMap();

  if (rootMap.contains(scopeStr)) {
    QStringList scopes = rootMap.value(scopeStr).toString().trimmed().split(',');
    foreach (QString scope, scopes) {
      scope = scope.trimmed();
      if (s_scopeMetadataMap.count(scope) == 0) {
        s_scopeMetadataMap.insert(
            std::make_pair(scope, std::move(std::unique_ptr<Metadata>(new Metadata(scope)))));
      }
      Metadata* metadata = s_scopeMetadataMap[scope].get();

      QVariantMap rootMap = root.toMap();

      // name
      if (rootMap.contains(nameStr)) {
        metadata->setName(rootMap.value(nameStr).toString());
      }

      // settings
      if (rootMap.contains(settingsStr)) {
        QVariantMap settingMap = rootMap.value(settingsStr).toMap();
        if (settingMap.contains(increaseIndentPatternStr)) {
          metadata->setIncreaseIndentPattern(settingMap.value(increaseIndentPatternStr).toString());
        }
        if (settingMap.contains(decreaseIndentPatternStr)) {
          metadata->setDecreateIndentPattern(settingMap.value(decreaseIndentPatternStr).toString());
        }
        if (settingMap.contains(bracketIndentNextLinePatternStr)) {
          metadata->setBracketIndentNextLinePattern(
              settingMap.value(bracketIndentNextLinePatternStr).toString());
        }
        if (settingMap.contains(disableIndentNextLinePatternStr)) {
          metadata->setDisableIndentNextLinePattern(
              settingMap.value(disableIndentNextLinePatternStr).toString());
        }
        if (settingMap.contains(unIndentedLinePatternStr)) {
          metadata->setUnIndentedLinePattern(settingMap.value(unIndentedLinePatternStr).toString());
        }
      }
    }
  }
}

}  // namespace core
