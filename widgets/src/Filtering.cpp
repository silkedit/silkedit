#include <QLabel>
#include <QAbstractButton>
#include <QLayout>
#include <QRegularExpression>

#include "Filtering.h"
#include "core/Config.h"

using core::Config;

namespace {

bool match(const QString& filterText, QObject* object) {
  if (QLabel* label = qobject_cast<QLabel*>(object)) {
    return Filtering::contains(label->text(), filterText);
  } else if (QAbstractButton* button = qobject_cast<QAbstractButton*>(object)) {
    return Filtering::contains(button->text(), filterText);
  } else if (QLayout* layout = qobject_cast<QLayout*>(object)) {
    for (int i = 0; i < layout->count(); i++) {
      if (layout->itemAt(i)->widget() && match(filterText, layout->itemAt(i)->widget())) {
        return true;
      }
    }
  }

  return false;
}

void setVisible(QObject* object, bool visible) {
  if (QLabel* label = qobject_cast<QLabel*>(object)) {
    label->setVisible(visible);
    if (label->buddy()) {
      label->buddy()->setVisible(visible);
    }
  } else if (QWidget* widget = qobject_cast<QWidget*>(object)) {
    widget->setVisible(visible);
  } else if (QLayout* layout = qobject_cast<QLayout*>(object)) {
    for (int i = 0; i < layout->count(); i++) {
      if (layout->itemAt(i)->widget()) {
        layout->itemAt(i)->widget()->setVisible(visible);
      }
    }
  }
}
}

bool Filtering::contains(const QString& s1, const QString& s2) {
  if (s2.isEmpty())
    return true;

  // Use QString::contains(const QString&) for Japanese text because it doesn't have a word boundary
  if (QLocale(Config::singleton().locale()).language() == QLocale::Japanese) {
    return s1.contains(s2, Qt::CaseInsensitive);
  } else {
    return s1.contains(QRegularExpression(R"(\b)" + s2, QRegularExpression::CaseInsensitiveOption));
  }
}

bool Filtering::filter(const QString& filterText) {
  return filter(filterText, m_objects);
}

void Filtering::resetFilter() {
  filter("", m_objects);
}

bool Filtering::filter(const QString& filterText, QObjectList objects) {
  bool hasMatch = false;
  for (QObject* object : objects) {
    if (match(filterText, object)) {
      hasMatch = true;
      setVisible(object, true);
    } else {
      setVisible(object, false);
    }
  }

  return hasMatch;
}
