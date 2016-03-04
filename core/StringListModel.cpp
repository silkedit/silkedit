#include <algorithm>

#include "StringListModel.h"

namespace core {

StringListModel::StringListModel(QObject* parent) : QStringListModel(parent) {}

QStringList core::StringListModel::stringList() const {
  return QStringListModel::stringList();
}

void StringListModel::setStringList(const QVariantList& variants) {
  if (std::all_of(variants.constBegin(), variants.constEnd(),
                  [](QVariant var) { return var.canConvert<QString>(); })) {
    QStringList strings;
    for (auto var : variants) {
      strings.append(var.toString());
    }
    QStringListModel::setStringList(strings);
  }
}

}  // namespace core
