#pragma once

#include <QtCore>
#include <QDomElement>

namespace core {

class PListParser {
 public:
  static QVariant parsePList(QIODevice* device);

 private:
  static QVariant parseElement(const QDomElement& e);
  static QVariantList parseArrayElement(const QDomElement& node);
  static QVariantMap parseDictElement(const QDomElement& element);
};

}  // namespace core
