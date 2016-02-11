#include <boost/optional.hpp>
#include <tuple>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>

#include "Document.h"
#include "LineSeparator.h"
#include "Config.h"
#include "LanguageParser.h"
#include "Regexp.h"
#include "SyntaxHighlighter.h"

namespace core {

namespace {

boost::optional<std::tuple<QString, Encoding, QString, BOM>> load(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec;
  auto guessedEncoding = Encoding::guessEncoding(contentBytes);
  codec = guessedEncoding.codec();
  const QString& text = codec->toUnicode(contentBytes);
  const LineSeparator& separator = LineSeparator::guess(text);
  const BOM& bom = BOM::guessBOM(contentBytes);
  return std::make_tuple(text, guessedEncoding, separator.separatorStr(), bom);
}

boost::optional<std::tuple<QString, QString, BOM>> load(const QString& path,
                                                        const Encoding& encoding) {
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return boost::none;

  QByteArray contentBytes = file.readAll();
  QTextCodec* codec = encoding.codec();
  const QString& text = codec->toUnicode(contentBytes);
  const BOM& bom = BOM::guessBOM(contentBytes);
  return std::make_tuple(text, LineSeparator::guess(text).separatorStr(), bom);
}
}

Document::Document(const QString& path,
                   const QString& text,
                   const Encoding& encoding,
                   const QString& separator,
                   const BOM& bom)
    : QTextDocument(text),
      m_path(path),
      m_encoding(encoding),
      m_lineSeparator(separator),
      m_bom(bom),
      m_syntaxHighlighter(nullptr) {
  init();

  int dotPos = path.lastIndexOf('.');
  if (dotPos >= 0) {
    QString ext = path.mid(dotPos + 1);
    qDebug("ext: %s", qPrintable(ext));
    // todo: support firstLineMatch
    setupSyntaxHighlighter(LanguageProvider::languageFromExtension(ext), toPlainText());
  } else {
    qDebug("extension not found. path: %s", qPrintable(path));
  }
}

void Document::init() {
  // This font is used for an empty line because SyntaxHilighter can't set font in am empty line
  setDefaultFont(Config::singleton().font());
  setupLayout();
  connect(&Config::singleton(), &Config::fontChanged, this, &QTextDocument::setDefaultFont);
}

Document::Document()
    : m_encoding(Encoding::defaultEncoding()),
      m_lineSeparator(LineSeparator::defaultLineSeparator().separatorStr()),
      m_bom(BOM::defaultBOM()),
      m_syntaxHighlighter(nullptr) {
  init();
  setupSyntaxHighlighter(LanguageProvider::defaultLanguage());
}

void Document::setupLayout() {
  QPlainTextDocumentLayout* layout = new QPlainTextDocumentLayout(this);
  setDocumentLayout(layout);
}

void Document::setupSyntaxHighlighter(Language* lang, const QString& text) {
  m_lang.reset(lang);
  if (m_lang) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, text);
    m_syntaxHighlighter = new SyntaxHighlighter(this, parser);
  } else {
    qDebug("lang is null");
  }
}

Document::~Document() {
  qDebug("~Document");
}

Document* Document::create(const QString& path) {
  //  qDebug() << "Docment::create" << "path" << path;
  if (const boost::optional<std::tuple<QString, Encoding, QString, BOM>> textAndEncAndSeparator =
          load(path)) {
    return new Document(path, std::get<0>(*textAndEncAndSeparator),
                        std::get<1>(*textAndEncAndSeparator), std::get<2>(*textAndEncAndSeparator),
                        std::get<3>(*textAndEncAndSeparator));
  } else {
    return nullptr;
  }
}

Document* Document::createBlank() {
  return new Document();
}

void Document::setPath(const QString& path) {
  m_path = path;
  emit pathUpdated(path);
}

void Document::setLanguage(const QString& scopeName) {
  qDebug("setLanguage: %s", qPrintable(scopeName));
  Language* newLang = LanguageProvider::languageFromScope(scopeName);
  if (m_lang.get() == newLang || (m_lang && newLang && *m_lang == *newLang)) {
    qDebug("lang is already %s", qPrintable(scopeName));
    return;
  }

  m_lang.reset(newLang);
  if (m_lang && m_syntaxHighlighter) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText());
    m_syntaxHighlighter->setParser(parser);
    if (m_syntaxHighlighter) {
      m_syntaxHighlighter->rehighlight();
    }
  }
  emit languageChanged(scopeName);
}

void Document::setEncoding(const Encoding& encoding) {
  if (m_encoding != encoding) {
    m_encoding = encoding;
    emit encodingChanged(encoding);
  }
}

void Document::setLineSeparator(const QString& lineSeparator) {
  if (m_lineSeparator != lineSeparator) {
    m_lineSeparator = lineSeparator;
    emit lineSeparatorChanged(lineSeparator);
  }
}

void Document::setBOM(const BOM& bom) {
  if (m_bom != bom) {
    m_bom = bom;
    emit bomChanged(bom);
  }
}

std::unique_ptr<Regexp> Document::createRegexp(const QString& subString,
                                               Document::FindFlags options) const {
  bool isCaseSensitive = options & FindFlag::FindCaseSensitively;
  bool isRegex = options & FindFlag::FindRegex;
  bool isWholeWord = options & FindFlag::FindWholeWords;

  QString str = isRegex ? subString : Regexp::escape(subString);

  if (isWholeWord) {
    str = "\\b" + str + "\\b";
  }

  if (isCaseSensitive) {
    str = "(?-i)" + str;
  } else {
    str = "(?i)" + str;
  }

  return Regexp::compile(str);
}

QTextCursor Document::find(const QString& subString,
                           int from,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (subString.isEmpty()) {
    return QTextCursor();
  }

  std::unique_ptr<Regexp> regexp = createRegexp(subString, options);

  if (regexp) {
    return find(regexp.get(), from, begin, end, options);
  } else {
    return QTextCursor();
  }
}

QTextCursor Document::find(const QString& subString,
                           const QTextCursor& cursor,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (cursor.isNull())
    return QTextCursor();

  int pos;
  if (options & FindFlag::FindBackward) {
    if (cursor.selectedText().size() == 0) {
      if (cursor.atStart()) {
        QTextCursor newCursor(cursor);
        newCursor.movePosition(QTextCursor::End);
        pos = newCursor.position();
      } else {
        pos = cursor.selectionStart() - 1;
      }
    } else {
      pos = cursor.selectionStart();
    }
  } else {
    if (cursor.selectedText().size() == 0) {
      if (cursor.atEnd()) {
        pos = 0;
      } else {
        pos = cursor.selectionEnd() + 1;
      }
    } else {
      pos = cursor.selectionEnd();
    }
  }

  return find(subString, pos, begin, end, options);
}

QVector<Region> Document::findAll(const QString& text,
                                  int begin,
                                  int end,
                                  Document::FindFlags flags) const {
  return findAll(createRegexp(text, flags).get(), begin, end);
}

QVector<Region> Document::findAll(const Regexp* expr, int begin, int end) const {
  qDebug("findAll: %s, begin: %d, end: %d", qPrintable(expr->pattern()), begin, end);
  auto indicesList = expr->findAllStringSubmatchIndex(toPlainText(), begin, end);
  QVector<Region> regions(indicesList.size());
  if (!indicesList.isEmpty()) {
    std::transform(indicesList.begin(), indicesList.end(), regions.begin(),
                   [=](QVector<int> indices) { return Region(indices[0], indices[1]); });
    return regions;
  }

  return QVector<Region>();
}

QTextCursor Document::find(const Regexp* expr,
                           int from,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  bool isBackward = options.testFlag(FindFlag::FindBackward);
  qDebug("find: %s, back: %d, from: %d, begin: %d, end: %d", qPrintable(expr->pattern()),
         (options.testFlag(FindFlag::FindBackward)), from, begin, end);
  QVector<int> indices;
  if (isBackward) {
    indices = expr->findStringSubmatchIndex(toPlainText(), begin, from, isBackward);
  } else {
    indices = expr->findStringSubmatchIndex(toPlainText(), from, end, isBackward);
  }
  if (indices.size() > 1) {
    int endPos = indices.at(1);

    if (endPos >= 0) {
      int startPos = indices.at(0);
      QTextCursor resultCursor(docHandle(), startPos);
      resultCursor.setPosition(endPos, QTextCursor::KeepAnchor);
      return resultCursor;
    }
  }

  return QTextCursor();
}

QTextCursor Document::find(const Regexp* expr,
                           const QTextCursor& cursor,
                           int begin,
                           int end,
                           Document::FindFlags options) const {
  if (cursor.isNull())
    return QTextCursor();

  int pos;
  if (options & QTextDocument::FindBackward) {
    pos = cursor.selectionStart();
  } else {
    pos = cursor.selectionEnd();
  }
  return find(expr, pos, begin, end, options);
}

QString Document::scopeName(int pos) const {
  return m_syntaxHighlighter ? m_syntaxHighlighter->scopeName(pos) : "";
}

QString Document::scopeTree() const {
  return m_syntaxHighlighter ? m_syntaxHighlighter->scopeTree() : "";
}

void Document::reload() {
  if (const boost::optional<std::tuple<QString, Encoding, QString, BOM>>
          textAndEncAndSeparatorAndBOM = load(m_path)) {
    setPlainText(std::get<0>(*textAndEncAndSeparatorAndBOM));
    setEncoding(std::get<1>(*textAndEncAndSeparatorAndBOM));
    setLineSeparator(std::get<2>(*textAndEncAndSeparatorAndBOM));
    setBOM(std::get<3>(*textAndEncAndSeparatorAndBOM));
    setModified(false);
    emit modificationChanged(false);
  }
}

void Document::reload(const Encoding& encoding) {
  if (const boost::optional<std::tuple<QString, QString, BOM>> textAndSeparatorAndBOM =
          load(m_path, encoding)) {
    setPlainText(std::get<0>(*textAndSeparatorAndBOM));
    setEncoding(encoding);
    setLineSeparator(std::get<1>(*textAndSeparatorAndBOM));
    setBOM(std::get<2>(*textAndSeparatorAndBOM));
    setModified(false);
    emit modificationChanged(false);
  }
}

}  // namespace core
