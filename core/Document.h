#pragma once

#include <memory>
#include <QTextDocument>
#include <QTextOption>

#include "macros.h"
#include "Encoding.h"
#include "BOM.h"
#include "Region.h"

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

  // Don't call this except DocumentManager
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

  boost::optional<Region> find(const QString& subString,
                               int from = 0,
                               int begin = 0,
                               int end = -1,
                               FindFlags options = 0) const;

  boost::optional<Region> find(const Regexp* expr,
                               int from = 0,
                               int begin = 0,
                               int end = -1,
                               FindFlags options = 0) const;

  QVector<core::Region> findAll(const QString& text,
                                int begin,
                                int end,
                                core::Document::FindFlags flags) const;

  QVector<core::Region> findAll(const Regexp* expr, int begin, int end) const;

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
  int tabWidth(Language *lang);

 signals:
  void pathUpdated(const QString& path);
  void languageChanged(const QString& scopeName);
  void encodingChanged(const Encoding& encoding);
  void lineSeparatorChanged(const QString& lineSeparator);
  void bomChanged(const BOM& bom);
  void parseFinished();

 public slots:
  QTextOption defaultTextOption() const;
  void setDefaultTextOption(const QTextOption& option);

 private:
  friend class DocumentTest;

  QString m_path;
  std::unique_ptr<Language> m_lang;
  Encoding m_encoding;
  QString m_lineSeparator;
  BOM m_bom;
  SyntaxHighlighter* m_syntaxHighlighter;
  QString m_tabWidthKey;

  Document(const QString& path,
           const QString& text,
           const Encoding& encoding,
           const QString& separator,
           const BOM& bom);
  Document();

  void setupLayout();
  void setupSyntaxHighlighter(std::unique_ptr<Language> lang, const QString& text = "");
  void init();
  std::unique_ptr<Regexp> createRegexp(const QString& subString, Document::FindFlags options) const;
  void setShowTabsAndSpaces(bool showTabsAndSpaces);
  void setTabWidth(int tabWidth);
  void setTabWidth();
};

}  // namespace core

Q_DECLARE_METATYPE(core::Document*)
