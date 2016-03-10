#include <boost/optional.hpp>
#include <tuple>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QDir>

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

  Language* lang = nullptr;
  int from = 0, dotPos = -1;

  while (!lang) {
    dotPos = path.indexOf('.', from);
    if (dotPos >= 0) {
      const QString& ext = path.mid(dotPos + 1);
      qDebug() << "ext:" << ext;
      // todo: support firstLineMatch
      lang = LanguageProvider::languageFromExtension(ext);
      if (!lang) {
        from = dotPos + 1;
      }
    } else {
      const auto& filename = path.mid(path.lastIndexOf('/') + 1);
      lang = LanguageProvider::languageFromExtension(filename);
      if (!lang) {
        lang = LanguageProvider::defaultLanguage();
        Q_ASSERT(lang);
      }
    }
  }

  Q_ASSERT(lang);
  setupSyntaxHighlighter(std::move(std::unique_ptr<Language>(lang)), toPlainText());
  setTabWidth();
}

int Document::tabWidth(Language* lang) {
  return lang ? Config::singleton().tabWidth(lang->scopeName) : Config::singleton().tabWidth();
}

void Document::setTabWidth() {
  QString scopeName = m_lang ? m_lang->scopeName : "";
  m_tabWidthKey = Config::singleton().tabWidthKey(scopeName);
  setTabWidth(tabWidth(m_lang.get()));
}

void Document::setTabWidth(int tabWidth) {
  qreal width = QFontMetricsF(Config::singleton().font()).width(QLatin1Char(' '));
  QTextOption option = defaultTextOption();
  option.setTabStop(width * tabWidth);
  setDefaultTextOption(option);
}

void Document::setShowTabsAndSpaces(bool showTabsAndSpaces) {
  auto option = defaultTextOption();
  QTextOption::Flags flags;
  if (showTabsAndSpaces) {
    flags = option.flags() | QTextOption::ShowTabsAndSpaces;
  } else {
    flags = option.flags() & ~QTextOption::ShowTabsAndSpaces;
  }
  option.setFlags(flags);
  setDefaultTextOption(option);
}

void Document::init() {
  // This font is used for an empty line because SyntaxHilighter can't set font in am empty line
  setDefaultFont(Config::singleton().font());
  setupLayout();
  setShowTabsAndSpaces(Config::singleton().showTabsAndSpaces());

  connect(&Config::singleton(), &Config::fontChanged, this, [=](QFont font) {
    setDefaultFont(font);
    setTabWidth();
  });
  connect(&Config::singleton(), &Config::showTabsAndSpacesChanged, this,
          &Document::setShowTabsAndSpaces);
  connect(&Config::singleton(), &Config::configChanged, this,
          [=](const QString& key, QVariant, QVariant newValue) {
            if (key == m_tabWidthKey && newValue.canConvert<int>()) {
              setTabWidth(newValue.value<int>());
            }
          });
}

Document::Document()
    : m_encoding(Encoding::defaultEncoding()),
      m_lineSeparator(LineSeparator::defaultLineSeparator().separatorStr()),
      m_bom(BOM::defaultBOM()),
      m_syntaxHighlighter(nullptr) {
  init();
  setupSyntaxHighlighter(std::move(std::unique_ptr<Language>(LanguageProvider::defaultLanguage())));
}

void Document::setupLayout() {
  QPlainTextDocumentLayout* layout = new QPlainTextDocumentLayout(this);
  setDocumentLayout(layout);
}

void Document::setupSyntaxHighlighter(std::unique_ptr<Language> lang, const QString& text) {
  m_lang = std::move(lang);
  if (m_lang) {
    std::unique_ptr<LanguageParser> parser(LanguageParser::create(m_lang->scopeName, text));
    m_syntaxHighlighter = new SyntaxHighlighter(
        this, std::move(parser), Config::singleton().theme(), Config::singleton().font());
    connect(m_syntaxHighlighter, &SyntaxHighlighter::parseFinished, this, &Document::parseFinished);
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
    if (LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText())) {
      m_syntaxHighlighter->setParser(*parser);
    }
  }
  setTabWidth();
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

boost::optional<Region> Document::find(const QString& subString,
                                       int from,
                                       int begin,
                                       int end,
                                       Document::FindFlags options) const {
  if (subString.isEmpty()) {
    return boost::none;
  }

  std::unique_ptr<Regexp> regexp = createRegexp(subString, options);

  if (regexp) {
    return find(regexp.get(), from, begin, end, options);
  } else {
    return boost::none;
  }
}

QVector<Region> Document::findAll(const QString& text,
                                  int begin,
                                  int end,
                                  Document::FindFlags flags) const {
  if (text.isEmpty()) {
    return QVector<Region>();
  }

  if (auto regexp = createRegexp(text, flags)) {
    return findAll(regexp.get(), begin, end);
  }

  return QVector<Region>();
}

QVector<Region> Document::findAll(const Regexp* expr, int begin, int end) const {
  if (!expr) {
    qWarning() << "expr is null";
    return QVector<Region>();
  }

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

boost::optional<Region> Document::find(const Regexp* expr,
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
      return Region(indices.at(0), endPos);
    }
  }

  return boost::none;
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

QTextOption Document::defaultTextOption() const {
  return QTextDocument::defaultTextOption();
}

void Document::setDefaultTextOption(const QTextOption& option) {
  QTextDocument::setDefaultTextOption(option);
}

}  // namespace core
