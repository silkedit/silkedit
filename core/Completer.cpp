#include "Completer.h"

namespace core {

Completer::Completer(QObject* parent) : QCompleter(parent) {}

void Completer::setWidget(QWidget* widget) {
  QCompleter::setWidget(widget);
}

QWidget* Completer::widget() const {
  return QCompleter::widget();
}

void Completer::setModel(QAbstractItemModel* c) {
  QCompleter::setModel(c);
}

QAbstractItemModel* Completer::model() const {
  return QCompleter::model();
}

void Completer::setCompletionMode(Completer::CompletionMode mode) {
  QCompleter::setCompletionMode(static_cast<QCompleter::CompletionMode>(mode));
}

QAbstractItemView* Completer::popup() const {
  return QCompleter::popup();
}

void Completer::setCaseSensitivity(Qt::CaseSensitivity caseSensitivity) {
  QCompleter::setCaseSensitivity(caseSensitivity);
}

void Completer::setModelSorting(Completer::ModelSorting sorting) {
  QCompleter::setModelSorting(static_cast<QCompleter::ModelSorting>(sorting));
}

int Completer::completionCount() const {
  return QCompleter::completionCount();
}

QString Completer::currentCompletion() const {
  return QCompleter::currentCompletion();
}

QAbstractItemModel* Completer::completionModel() const {
  return QCompleter::completionModel();
}

}  // namespace core
