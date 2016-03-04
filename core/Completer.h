#pragma once

#include <QCompleter>

#include "core/macros.h"

namespace core {

class Completer : public QCompleter {
  Q_OBJECT
  DISABLE_COPY(Completer)

 public:
  enum CompletionMode { PopupCompletion, UnfilteredPopupCompletion, InlineCompletion };
  Q_ENUM(CompletionMode)

  enum ModelSorting { UnsortedModel = 0, CaseSensitivelySortedModel, CaseInsensitivelySortedModel };
  Q_ENUM(ModelSorting)

  Q_INVOKABLE Completer(QObject* parent = nullptr);
  ~Completer() = default;
  DEFAULT_MOVE(Completer)

 public slots:
  void setWidget(QWidget* widget);
  QWidget* widget() const;

  void setModel(QAbstractItemModel* c);
  QAbstractItemModel* model() const;

  void setCompletionMode(CompletionMode mode);

  QAbstractItemView* popup() const;

  void setCaseSensitivity(Qt::CaseSensitivity caseSensitivity);

  void setModelSorting(ModelSorting sorting);

  int completionCount() const;

  QString currentCompletion() const;

  QAbstractItemModel* completionModel() const;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::Completer*)
