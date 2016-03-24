#include <QString>

#include "GrammerCondition.h"
#include "App.h"
#include "TextEdit.h"
#include "core/LanguageParser.h"

const QString GrammerCondition::name = "grammer";

QVariant GrammerCondition::value() {
  if (App::instance()->activeTextEdit()) {
    auto view = App::instance()->activeTextEdit();
    if (view->language()) {
      return QVariant::fromValue(view->language()->scopeName);
    }
  }
  return QVariant("");
}
