#include <QString>

#include "GrammerCondition.h"
#include "App.h"
#include "TextEditView.h"
#include "core/LanguageParser.h"

const QString GrammerCondition::name = "grammer";

QVariant GrammerCondition::keyValue() {
  if (App::instance()->activeTextEditView()) {
    auto view = App::instance()->activeTextEditView();
    if (view->language()) {
      return QVariant::fromValue(view->language()->scopeName);
    }
  }
  return QVariant("");
}
