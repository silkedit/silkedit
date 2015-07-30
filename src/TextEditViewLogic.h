#pragma once

#include <QString>
#include <QTextDocument>
#include <QTextCursor>

#include "macros.h"

class Regexp;

/**
 * @brief This class has business logics for TextEditView
 * Created mainly for unit testing
 */
class TextEditViewLogic {
  DISABLE_COPY_AND_MOVE(TextEditViewLogic)
 public:
  static void outdent(QTextDocument* doc, QTextCursor& cursor, int tabWidth);
  static bool isOutdentNecessary(Regexp* increaseIndentPattern,
                                 Regexp* decreaseIndentPattern,
                                 const QString& currentLineText,
                                 const QString& prevLineText,
                                 bool isAtBlockEnd,
                                 int tabWidth);

 private:
  TextEditViewLogic() = delete;
  ~TextEditViewLogic() = delete;
};
