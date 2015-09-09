#include <memory>
#include <QVector>

#include "TextEditViewLogic.h"
#include "Regexp.h"
#include "Metadata.h"

namespace {
int indentLength(const QString& str, int tabWidth) {
  int len = 0;
  std::unique_ptr<Regexp> regex(Regexp::compile(R"r(^\s+)r"));
  std::unique_ptr<QVector<int>> regions(regex->findStringSubmatchIndex(QStringRef(&str)));
  if (regions) {
    QString indentStr = str.left(regions->at(1));
    foreach (const QChar& ch, indentStr) {
      if (ch == '\t') {
        len += tabWidth;
      } else {
        len++;
      }
    }
  }

  return len;
}
}

void TextEditViewLogic::outdent(QTextDocument* doc, QTextCursor& cursor, int tabWidth) {
  bool moved = cursor.movePosition(QTextCursor::StartOfLine);
  if (!moved)
    return;

  QChar currentChar = doc->characterAt(cursor.position());
  if (currentChar == '\t') {
    cursor.deleteChar();
  } else if (currentChar == ' ') {
    int i = 0;
    while (!cursor.atBlockEnd() && i < tabWidth && doc->characterAt(cursor.position()) == ' ') {
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
      i++;
    }
    cursor.removeSelectedText();
  }
}

bool TextEditViewLogic::isOutdentNecessary(Regexp* increaseIndentPattern,
                                           Regexp* decreaseIndentPattern,
                                           const QString& currentLineText,
                                           const QString& prevLineText,
                                           bool isAtBlockEnd,
                                           int tabWidth) {
  if (!prevLineText.isEmpty()) {
    int currentLineIndentSize = indentLength(currentLineText, tabWidth);
    int prevLineIndentSize = indentLength(prevLineText, tabWidth);
    bool isPrevLineIncreasePattern = false;
    if (increaseIndentPattern && increaseIndentPattern->matches(prevLineText)) {
      isPrevLineIncreasePattern = true;
    }

    // Outdent in these cases
    // case 1 (previous line's indent level is equal to the indent level of current line)
    //     {
    //       int hoge = 0;
    //       } inserted here

    // case 2 (previous line matches increaseIndentPattern and the diff of indent level between
    // previous line and current line is exactly 1 indent level)
    //     {
    //       } inserted here
    bool isOutdentNecessary = false;
    if (isAtBlockEnd &&
        ((!isPrevLineIncreasePattern && currentLineIndentSize == prevLineIndentSize) ||
         (isPrevLineIncreasePattern && (currentLineIndentSize - prevLineIndentSize) == tabWidth))) {
      isOutdentNecessary = true;
    }

    if (isOutdentNecessary && decreaseIndentPattern &&
        decreaseIndentPattern->matches(currentLineText)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Indent one level
 * @param currentVisibleCursor
 */
void TextEditViewLogic::indentOneLevel(QTextCursor& currentVisibleCursor,
                                       bool indentUsingSpaces,
                                       int tabWidth) {
  QString indentStr = "\t";
  if (indentUsingSpaces) {
    indentStr = QString(tabWidth, ' ');
  }
  currentVisibleCursor.insertText(indentStr);
}

void TextEditViewLogic::indentCurrentLine(QTextDocument* doc,
                                          QTextCursor& cursor,
                                          const QString& prevLineText,
                                          const boost::optional<QString>& prevPrevLineText,
                                          Metadata* metadata,
                                          bool indentUsingSpaces,
                                          int tabWidth) {
  std::unique_ptr<Regexp> regex(Regexp::compile(R"r(^\s+)r"));
  std::unique_ptr<QVector<int>> regions(regex->findStringSubmatchIndex(QStringRef(&prevLineText)));
  // align the current line with the previous line
  if (regions) {
    cursor.insertText(prevLineText.left(regions->at(1)));
  }

  // check increaseIndentPattern for additional indent
  if (metadata) {
    bool indentNextLine = (metadata->increaseIndentPattern() &&
                           metadata->increaseIndentPattern()->matches(prevLineText)) ||
                          (metadata->bracketIndentNextLinePattern() &&
                           metadata->bracketIndentNextLinePattern()->matches(prevLineText));
    bool outdentNextLine = (metadata->bracketIndentNextLinePattern() && prevPrevLineText &&
                            metadata->bracketIndentNextLinePattern()->matches(*prevPrevLineText) &&
                            (!metadata->increaseIndentPattern() ||
                             !metadata->increaseIndentPattern()->matches(*prevPrevLineText)));
    if (indentNextLine) {
      TextEditViewLogic::indentOneLevel(cursor, indentUsingSpaces, tabWidth);
    } else if (outdentNextLine) {
      TextEditViewLogic::outdent(doc, cursor, tabWidth);
    }
  }
}
