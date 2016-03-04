#include <QTextBlock>

#include "TextCursor.h"

namespace core {

QTextBlock TextCursor::block() const {
  return m_wrapped.value<QTextCursor>().block();
}

bool TextCursor::movePosition(TextCursor::MoveOperation operation,
                              TextCursor::MoveMode mode,
                              int n) {
  // QVariant::value<QTextCursor>() returns a copy of m_wrapped, so we need to reassign it after
  // movePosition
  auto cursor = m_wrapped.value<QTextCursor>();
  auto result = cursor.movePosition(static_cast<QTextCursor::MoveOperation>(operation),
                                    static_cast<QTextCursor::MoveMode>(mode), n);
  m_wrapped = QVariant::fromValue(cursor);
  return result;
}

int TextCursor::position() const {
  return m_wrapped.value<QTextCursor>().position();
}

void TextCursor::setPosition(int pos, TextCursor::MoveMode m) {
  auto cursor = m_wrapped.value<QTextCursor>();
  cursor.setPosition(pos, static_cast<QTextCursor::MoveMode>(m));
  m_wrapped = QVariant::fromValue(cursor);
}

QString TextCursor::selectedText() const {
  return m_wrapped.value<QTextCursor>().selectedText();
}

void TextCursor::insertText(const QString& text) {
  auto cursor = m_wrapped.value<QTextCursor>();
  cursor.insertText(text);
  m_wrapped = QVariant::fromValue(cursor);
}

void TextCursor::removeSelectedText() {
  auto cursor = m_wrapped.value<QTextCursor>();
  cursor.removeSelectedText();
  m_wrapped = QVariant::fromValue(cursor);
}

void TextCursor::clearSelection() {
  auto cursor = m_wrapped.value<QTextCursor>();
  cursor.clearSelection();
  m_wrapped = QVariant::fromValue(cursor);
}

}  // namespace core
