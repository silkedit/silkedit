#pragma once

#include <QTextCursor>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

// Wrapper of QTextCursor
class TextCursor : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QTextCursor")

 public:
  // redefined enums
  enum MoveMode { MoveAnchor, KeepAnchor };
  Q_ENUM(MoveMode)

  enum MoveOperation {
    NoMove,

    Start,
    Up,
    StartOfLine,
    StartOfBlock,
    StartOfWord,
    PreviousBlock,
    PreviousCharacter,
    PreviousWord,
    Left,
    WordLeft,

    End,
    Down,
    EndOfLine,
    EndOfWord,
    EndOfBlock,
    NextBlock,
    NextCharacter,
    NextWord,
    Right,
    WordRight,

    NextCell,
    PreviousCell,
    NextRow,
    PreviousRow
  };
  Q_ENUM(MoveOperation)

  enum SelectionType { WordUnderCursor, LineUnderCursor, BlockUnderCursor, Document };
  Q_ENUM(SelectionType)

  TextCursor(QTextCursor cursor) { m_wrapped = QVariant::fromValue(cursor); }
  Q_INVOKABLE TextCursor() { m_wrapped = QVariant::fromValue(QTextCursor()); }
  ~TextCursor() = default;

  static bool customMovePosition(QTextCursor& cursor,
                                 QTextCursor::MoveOperation op,
                                 QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
                                 int n = 1);
  static void customSelect(QTextCursor &cursor, QTextCursor::SelectionType selection);

 public slots:
  QTextBlock block() const;
  bool movePosition(MoveOperation operation, MoveMode mode = MoveAnchor, int n = 1);
  int position() const;
  void setPosition(int pos, MoveMode m = MoveAnchor);
  QString selectedText() const;
  void insertText(const QString& text);
  void removeSelectedText();
  void clearSelection();
};

}  // namespace core

Q_DECLARE_METATYPE(core::TextCursor*)
Q_DECLARE_METATYPE(QTextCursor)
