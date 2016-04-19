#include <unicode/brkiter.h>
#include <unicode/uchriter.h>
#include <algorithm>
#include <QTextBlock>
#include <QDebug>

#include "TextCursor.h"
#include "IcuUtil.h"
#include "scoped_guard.h"
#include "Config.h"

namespace {
bool isAllWhiteSpaceChar(const QString& str) {
  return std::all_of(str.constBegin(), str.constEnd(), [](QChar ch) { return ch.isSpace(); });
}
}

namespace core {

// Move a cursor considering ICU word boundary.
// Original movePosition skips Japanese text entirely...
// http://userguide.icu-project.org/boundaryanalysis
bool TextCursor::customMovePosition(QTextCursor& cursor,
                                    QTextCursor::MoveOperation op,
                                    QTextCursor::MoveMode mode,
                                    int n) {
  switch (op) {
    case MoveOperation::NextWord: {
      QTextCursor newCursor(cursor);
      newCursor.clearSelection();
      QString text;
      bool result;
      do {
        result = newCursor.movePosition(op, QTextCursor::MoveMode::KeepAnchor, n);
        text = newCursor.selectedText();
      } while (result && isAllWhiteSpaceChar(text));

      UErrorCode status = U_ZERO_ERROR;
      auto boundary = BreakIterator::createWordInstance(
          IcuUtil::icuLocale(Config::singleton().locale()), status);
      scoped_guard guard([=] { delete boundary; });
      boundary->setText(IcuUtil::toIcuString(text));
      int32_t pos = 0;

      boundary->first();
      for (int i = 0; i < n;) {
        pos = boundary->next();

        if (pos == BreakIterator::DONE) {
          pos = 0;
          break;
        }

        newCursor.setPosition(cursor.position());
        newCursor.clearSelection();
        newCursor.setPosition(cursor.position() + pos, QTextCursor::MoveMode::KeepAnchor);

        if (!isAllWhiteSpaceChar(newCursor.selectedText())) {
          i++;
        }
      }

      cursor.setPosition(cursor.position() + pos, mode);
      return true;
    }
    case MoveOperation::PreviousWord: {
      QTextCursor newCursor(cursor);
      newCursor.clearSelection();
      QString text;
      bool result;
      do {
        result = newCursor.movePosition(op, QTextCursor::MoveMode::KeepAnchor, n);
        text = newCursor.selectedText();
      } while (result && isAllWhiteSpaceChar(text));

      UErrorCode status = U_ZERO_ERROR;
      auto boundary = BreakIterator::createWordInstance(
          IcuUtil::icuLocale(Config::singleton().locale()), status);
      scoped_guard guard([=] { delete boundary; });
      boundary->setText(IcuUtil::toIcuString(text));
      int32_t pos = 0;

      boundary->last();
      for (int i = 0; i < n;) {
        pos = boundary->previous();

        if (pos == BreakIterator::DONE) {
          pos = 0;
          break;
        }

        newCursor.setPosition(cursor.position());
        newCursor.clearSelection();
        newCursor.setPosition(cursor.position() - text.size() + pos, QTextCursor::MoveMode::KeepAnchor);

        if (!isAllWhiteSpaceChar(newCursor.selectedText())) {
          i++;
        }
      }

      cursor.setPosition(cursor.position() - text.size() + pos, mode);
      return true;
    }
    case MoveOperation::StartOfWord: {
      QTextCursor newCursor(cursor);
      newCursor.clearSelection();
      newCursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
      int lengthFromLeft = newCursor.selectionEnd() - newCursor.selectionStart();
      newCursor.clearSelection();
      newCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor, 2);
      const auto& text = newCursor.selectedText();
      UErrorCode status = U_ZERO_ERROR;
      auto boundary = BreakIterator::createWordInstance(
          IcuUtil::icuLocale(Config::singleton().locale()), status);
      scoped_guard guard([=] { delete boundary; });
      boundary->setText(IcuUtil::toIcuString(text));
      int32_t pos = 0;

      // When the cursor is at the end, move it as PreviousWord
      if (lengthFromLeft == text.size()) {
        return customMovePosition(cursor, QTextCursor::PreviousWord, mode, n);
      }

      // isBoundary has a side effect. The current position of the iterator is set
      // to the first boundary position at or following the specified offset.
      if (!boundary->isBoundary(lengthFromLeft)) {
        pos = boundary->previous();
        if (pos == BreakIterator::DONE) {
          pos = 0;
        }
      } else {
        return true;
      }
      cursor.setPosition(cursor.position() - lengthFromLeft + pos, mode);
      return true;
    }
    case MoveOperation::EndOfWord: {
      QTextCursor newCursor(cursor);
      newCursor.clearSelection();
      newCursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
      int lengthFromLeft = newCursor.selectionEnd() - newCursor.selectionStart();
      newCursor.clearSelection();
      newCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor, 2);
      const auto& text = newCursor.selectedText();
      UErrorCode status = U_ZERO_ERROR;
      auto boundary = BreakIterator::createWordInstance(
          IcuUtil::icuLocale(Config::singleton().locale()), status);
      scoped_guard guard([=] { delete boundary; });
      boundary->setText(IcuUtil::toIcuString(text));
      int32_t pos = 0;

      // When the cursor is at the start, move it as NextWord
      if (lengthFromLeft == 0) {
        return customMovePosition(cursor, QTextCursor::NextWord, mode, n);
      }

      if (!boundary->isBoundary(lengthFromLeft)) {
        pos = boundary->current();
        if (pos == BreakIterator::DONE) {
          pos = 0;
        }
      } else {
        return true;
      }
      cursor.setPosition(cursor.position() + (pos - lengthFromLeft), mode);
      return true;
    }
    default:
      return cursor.movePosition(op, mode, n);
  }
}

void TextCursor::customSelect(QTextCursor& cursor, QTextCursor::SelectionType selection) {
  switch (selection) {
    case SelectionType::WordUnderCursor:
      customMovePosition(cursor, QTextCursor::StartOfWord);
      // When the text is "単語単位" and the cursor is at position 2, movePositoin with EndOfWord
      // doesn't move it because that's the end of "単語".
      // So move the cursor right to select the Japanese word under it.
      cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
      customMovePosition(cursor, QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
      break;
    default:
      cursor.select(selection);
      break;
  }
}

QTextBlock TextCursor::block() const {
  return m_wrapped.value<QTextCursor>().block();
}

bool TextCursor::movePosition(TextCursor::MoveOperation operation,
                              TextCursor::MoveMode mode,
                              int n) {
  // QVariant::value<QTextCursor>() returns a copy of m_wrapped, so we need to reassign it after
  // movePosition
  auto cursor = m_wrapped.value<QTextCursor>();
  auto result = customMovePosition(cursor, static_cast<QTextCursor::MoveOperation>(operation),
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
