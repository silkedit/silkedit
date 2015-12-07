#pragma once

#include <memory>
#include <QTextDocument>

#include "macros.h"
#include "Encoding.h"
#include "BOM.h"

namespace core {

struct Language;
class Regexp;
class SyntaxHighlighter;

class Document : public QTextDocument {
  Q_OBJECT
  DISABLE_COPY(Document)

 public:
  enum class FindFlag {
    FindBackward = 0x00001,
    FindCaseSensitively = 0x00002,
    FindWholeWords = 0x00004,
    FindRegex = 0x00008,
    FindInSelection = 0x00010,
  };
  Q_DECLARE_FLAGS(FindFlags, FindFlag)

  ~Document();
  DEFAULT_MOVE(Document)

  static Document* create(const QString& path = "");
  static Document* createBlank();

  QString path() { return m_path; }
  void setPath(const QString& path);

  Language* language() { return m_lang.get(); }
  void setLanguage(const QString& scopeName);

  Encoding encoding() { return m_encoding; }
  void setEncoding(const Encoding& encoding);

  QString lineSeparator() { return m_lineSeparator; }
  void setLineSeparator(const QString& lineSeparator);

  BOM bom() { return m_bom; }
  void setBOM(const BOM& bom);

  QTextCursor find(const QString& subString,
                   int from = 0,
                   int begin = 0,
                   int end = -1,
                   FindFlags options = 0) const;
  QTextCursor find(const QString& subString,
                   const QTextCursor& from,
                   int begin = 0,
                   int end = -1,
                   FindFlags options = 0) const;
  QTextCursor find(const Regexp* expr,
                   int from = 0,
                   int begin = 0,
                   int end = -1,
                   FindFlags options = 0) const;
  QTextCursor find(const Regexp* expr,
                   const QTextCursor& cursor,
                   int begin = 0,
                   int end = -1,
                   FindFlags options = 0) const;
  QString scopeName(int pos) const;
  QString scopeTree() const;

  /**
   * @brief reload from a local file and guess its encoding
   */
  void reload();

  /**
   * @brief reload from a local file in the encoding
   * @param encoding
   */
  void reload(const Encoding& encoding);

 signals:
  void pathUpdated(const QString& path);
  void languageChanged(const QString& scopeName);
  void encodingChanged(const Encoding& encoding);
  void lineSeparatorChanged(const QString& lineSeparator);
  void bomChanged(const BOM& bom);

 private:
  QString m_path;
  std::unique_ptr<Language> m_lang;
  Encoding m_encoding;
  QString m_lineSeparator;
  BOM m_bom;
  SyntaxHighlighter* m_syntaxHighlighter;

  Document(const QString& path,
           const QString& text,
           const Encoding& encoding,
           const QString& separator,
           const BOM& bom);
  Document();

  void setupLayout();
  void setupSyntaxHighlighter(Language* lang, const QString& text = "");
  void init();
};

}  // namespace core
