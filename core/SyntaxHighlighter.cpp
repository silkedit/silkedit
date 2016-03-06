#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QDebug>

#include "SyntaxHighlighter.h"
#include "PListParser.h"
#include "Util.h"
#include "Config.h"
#include "Theme.h"

namespace core {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc,
                                     std::unique_ptr<LanguageParser> parser,
                                     Theme* theme,
                                     QFont font)
    : QSyntaxHighlighter(doc), m_parser(*parser), m_theme(theme) {
  Q_ASSERT(parser);

  /*
   setDocument creates connection below which calls highlightBlock in _q_reformatBlocks by
   contentsChange signal, but we want to override this behavior, so disconnect it.
   connect(d->doc, SIGNAL(contentsChange(int,int,int)),
     this, SLOT(_q_reformatBlocks(int,int,int)));
   */
  disconnect(doc, SIGNAL(contentsChange(int, int, int)), this,
             SLOT(_q_reformatBlocks(int, int, int)));

  connect(doc, &QTextDocument::contentsChange, this, &SyntaxHighlighter::updateNode);
  connect(&Config::singleton(), &Config::themeChanged, this, &SyntaxHighlighter::changeTheme);
  connect(&Config::singleton(), &Config::fontChanged, this, &SyntaxHighlighter::changeFont);

  if (m_theme) {
    m_theme->setFont(font);
  }

  QMetaObject::invokeMethod(&SyntaxHighlighterThread::singleton(), "parse", Qt::QueuedConnection,
                            Q_ARG(SyntaxHighlighter*, this), Q_ARG(LanguageParser, *m_parser));
}

SyntaxHighlighter::~SyntaxHighlighter() {
  qDebug("~SyntaxHighlighter");
}

void SyntaxHighlighter::setParser(LanguageParser parser) {
  m_parser = parser;
  QMetaObject::invokeMethod(&SyntaxHighlighterThread::singleton(), "parse", Qt::QueuedConnection,
                            Q_ARG(SyntaxHighlighter*, this), Q_ARG(LanguageParser, *m_parser));
}

Region SyntaxHighlighter::scopeExtent(int point) {
  updateScope(point);
  if (m_lastScopeNode) {
    return m_lastScopeNode->region;
  }
  return Region();
}

QString SyntaxHighlighter::scopeName(int point) {
  updateScope(point);
  return m_lastScopeName;
}

QString SyntaxHighlighter::scopeTree() {
  return m_rootNode ? m_rootNode->toString(document()->toPlainText()) : "";
}

void SyntaxHighlighter::highlight(const Region& region) {
  QMetaObject::invokeMethod(&SyntaxHighlighterThread::singleton(), "parse", Qt::QueuedConnection,
                            Q_ARG(SyntaxHighlighter*, this), Q_ARG(LanguageParser, *m_parser),
                            Q_ARG(QList<Node>, m_rootNode->children), Q_ARG(Region, region));
}

void SyntaxHighlighter::updateNode(int position, int charsRemoved, int charsAdded) {
  qDebug("contentsChange(pos: %d, charsRemoved: %d, charsAdded: %d)", position, charsRemoved,
         charsAdded);

  if (!document()) {
    qWarning() << "document is null";
    return;
  }

  // position is the position after removal happeened, so we need +charsRemoved
  //
  // NOTE: When pasting a text in an empty document, charsRemoved becomes 1 and charsAdded is the
  // actual charsAdded + 1 because of this bug.
  // We need to decrement them by 1.
  // https://bugreports.qt.io/browse/QTBUG-3495
  if (m_rootNode && m_rootNode->region.isEmpty() && charsRemoved == 1) {
    charsRemoved--;
    charsAdded--;
  }

  int delta = charsAdded - charsRemoved;

  if (m_rootNode) {
    m_rootNode->adjust(position + charsRemoved, delta);
  }

  //   We need to extend affectedRegion to the region from the beginning of the line at beginPos
  //   to the end of the line at endPos to support look ahead and behind regex.
  //   e.g.

  //   text:
  //     StatusBar QComboBox::down-arrow {
  //        /*image: url(noimg);*/

  //   pattern:
  //      <key>begin</key>
  //      <string>\s*(?=[:.*#a-zA-Z])</string>
  //      <key>end</key>
  //      <string>(?=[/@{)])</string>
  //      <key>name</key>
  //      <string>meta.selector.css</string>

  //   scope:
  //     0-32: "meta.selector.css" - Data: "StatusBar QComboBox::down-arrow "

  //   When we delete '{' at the end of line, beginPos is 32 and endPos is 33.
  //   In this case, [0-32) is not updated without expansion because [0-32) doesn't intersect
  //   [32-33).
  //   But we need to update [0-32) because its end pattern has /(?=/ and end pattern should match
  //   with '/' at pos 36

  int beginPos, endPos;

  beginPos = position;
  endPos = position + qMax(charsRemoved, charsAdded);

  if (delta > 0) {
    // When a text is added, we need to get begin and end pos based on the current document.
    m_parser->setText(document()->toPlainText());
    beginPos = m_parser->beginOfLine(beginPos);
    endPos = m_parser->endOfLine(endPos);

  } else {
    // When a text is removed, we need to get begin and end pos based on the text before removal
    beginPos = m_parser->beginOfLine(beginPos);
    endPos = m_parser->endOfLine(endPos);
    m_parser->setText(document()->toPlainText());
  }

  if (beginPos < 0 || endPos < 0) {
    qWarning() << "invalid begin or end position. beginPos" << beginPos << "endPos" << endPos;
    return;
  }

  highlight(Region(beginPos, endPos));
}

void SyntaxHighlighter::fullParseFinished(RootNode node) {
  m_rootNode = node;
  m_lastScopeNode = boost::none;
  rehighlight();
  emit parseFinished();
}

void SyntaxHighlighter::partialParseFinished(QList<Node> newNodes, Region region) {
  Region affectedRegion(region);
  if (newNodes.size() > 0) {
    // Extend affectedRegion by considering newNodes
    qDebug() << "affectedRegion:" << affectedRegion;
    affectedRegion.setBegin(qMin(affectedRegion.begin(), newNodes[0].region.begin()));
    affectedRegion.setEnd(qMax(affectedRegion.end(), newNodes[newNodes.size() - 1].region.end()));
  }

  m_rootNode->removeChildren(affectedRegion);
  m_rootNode->addChildren(newNodes);
  m_rootNode->sortChildren();

  qDebug("new children.size: %d", (int)m_rootNode->children.size());
  //  qDebug().noquote() << *this;

  //  qDebug().noquote() << "affectedRegion:" << affectedRegion;

  QTextBlock affectedBlock = document()->findBlock(affectedRegion.begin());
  while (affectedBlock.isValid() && affectedBlock.position() < affectedRegion.end()) {
    rehighlightBlock(affectedBlock);
    affectedBlock = affectedBlock.next();
  }

  m_lastScopeNode = boost::none;
  m_lastScopeBuf.clear();
  m_lastScopeName = "";
  emit parseFinished();
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
  if (!m_theme) {
    //    qDebug("theme is null");
    return;
  }

  int posInDoc = currentBlock().position();
  //  qDebug("highlightBlock. text: %s. current block pos: %d", qPrintable(text), posInDoc);

  for (int posInText = 0; posInText < text.length();) {
    updateScope(posInDoc + posInText);
    if (!m_lastScopeNode) {
      //      qDebug("lastScopeNode is null. after updateScope(%d)", posInDoc + posInText);
      return;
    }

    QTextCharFormat* format = m_theme->getFormat(m_lastScopeName);
    if (format) {
      if (m_lastScopeNode->isLeaf()) {
        Region region = m_lastScopeNode->region;
        int length = region.end() - (posInDoc + posInText);
        //      qDebug("%d - %d  %s", region.begin(), region.end(), qPrintable(m_lastScopeName));
        //        qDebug("setFormat(%d, %d, %s",
        //               i,
        //               qMin(text.length(), region.length()),
        //               qPrintable(format->foreground().color().name()));
        setFormat(posInText, qMin(text.length(), length), *format);
        posInText += length;
      } else {
        //        qDebug("setFormat(%d, %d, %s",
        //               i,
        //               1,
        //               qPrintable(format->foreground().color().name()));
        setFormat(posInText, 1, *format);
        posInText++;
      }
    } else {
      qDebug("format not found for %s", qPrintable(m_lastScopeName));
      posInText++;
    }
  }
}

boost::optional<Node> SyntaxHighlighter::findScope(const Region& search, const Node& node) {
  size_t idx = Util::binarySearch(node.children.size(), [search, node](size_t i) {
    return node.children[i].region.begin() >= search.begin() ||
           node.children[i].region.fullyCovers(search);
  });

  while (static_cast<int>(idx) < node.children.size()) {
    auto child = node.children[idx];
    if (child.region.begin() > search.end()) {
      break;
    }
    if (child.region.fullyCovers(search)) {
      if (!node.name.isEmpty() && node != m_lastScopeNode) {
        if (m_lastScopeBuf.length() > 0) {
          m_lastScopeBuf.append(' ');
        }
        m_lastScopeBuf.append(node.name);
      }
      return findScope(search, node.children[idx]);
    }
    idx++;
  }

  if (node != m_lastScopeNode && node.region.fullyCovers(search)) {
    if (!node.name.isEmpty()) {
      if (m_lastScopeBuf.length() > 0) {
        m_lastScopeBuf.append(' ');
      }
      m_lastScopeBuf.append(node.name);
    }
    return node;
  }

  return boost::none;
}

void SyntaxHighlighter::updateScope(int point) {
  //  qDebug("updateScope(point: %d)", point);

  if (!m_rootNode) {
    //    qDebug("root node is null");
    return;
  }

  Region search(point, point + 1);
  if (m_lastScopeNode && m_lastScopeNode->region.fullyCovers(search)) {
    if (m_lastScopeNode->children.size() != 0) {
      auto no = findScope(search, *m_lastScopeNode);
      if (no && no != m_lastScopeNode) {
        m_lastScopeNode = no;
        m_lastScopeName = QString(m_lastScopeBuf);
      }
    }
    return;
  }

  m_lastScopeNode = boost::none;
  m_lastScopeBuf.clear();
  if (m_rootNode) {
    m_lastScopeNode = findScope(search, *m_rootNode);
  }
  m_lastScopeName = QString(m_lastScopeBuf);
}

void SyntaxHighlighter::changeTheme(Theme* theme) {
  if (m_theme && m_theme->font()) {
    theme->setFont(*m_theme->font());
  }
  m_theme = theme;
  rehighlight();
}

void SyntaxHighlighter::changeFont(const QFont& font) {
  if (m_theme) {
    m_theme->setFont(font);
    rehighlight();
  }
}

QString SyntaxHighlighter::asHtml() {
  // Create a new document from all the selected text document.
  QTextCursor cursor(document());
  cursor.select(QTextCursor::Document);
  std::unique_ptr<QTextDocument> tempDocument(new QTextDocument);
  Q_ASSERT(tempDocument);
  QTextCursor tempCursor(tempDocument.get());

  tempCursor.insertFragment(cursor.selection());
  tempCursor.select(QTextCursor::Document);
  // Set the default foreground for the inserted characters.
  QTextCharFormat textfmt = tempCursor.charFormat();
  textfmt.setForeground(Qt::gray);
  tempCursor.setCharFormat(textfmt);

  // Apply the additional formats set by the syntax highlighter
  QTextBlock start = document()->findBlock(cursor.selectionStart());
  QTextBlock end = document()->findBlock(cursor.selectionEnd());
  end = end.next();
  const int selectionStart = cursor.selectionStart();
  const int endOfDocument = tempDocument->characterCount() - 1;
  for (QTextBlock current = start; current.isValid() && current != end; current = current.next()) {
    const QTextLayout* layout(current.layout());

    foreach (const QTextLayout::FormatRange& range, layout->additionalFormats()) {
      const int start = current.position() + range.start - selectionStart;
      const int end = start + range.length;
      if (end <= 0 || start >= endOfDocument)
        continue;
      tempCursor.setPosition(qMax(start, 0));
      tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
      tempCursor.setCharFormat(range.format);
    }
  }

  // Reset the user states since they are not interesting
  for (QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
    block.setUserState(-1);

  // Make sure the text appears pre-formatted, and set the background we want.
  tempCursor.select(QTextCursor::Document);
  QTextBlockFormat blockFormat = tempCursor.blockFormat();
  blockFormat.setNonBreakableLines(true);
  blockFormat.setBackground(Qt::black);
  tempCursor.setBlockFormat(blockFormat);

  // Finally retreive the syntax higlighted and formatted html.
  return tempCursor.selection().toHtml();
}

void SyntaxHighlighterThread::quit() {
  if (m_activeParser && m_activeParser->isParsing()) {
    m_activeParser->cancel();
    m_thread->wait(300);
  }
  m_thread->quit();
}

void SyntaxHighlighterThread::parse(SyntaxHighlighter* highlighter, LanguageParser parser) {
  if (highlighter) {
    if (m_activeParser && m_activeParser->isParsing()) {
      qDebug() << "Start full parsing with a new text";
      m_activeParser->cancel();
      // Start full parsing with a new text
      QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
      QTimer::singleShot(0, this, [=] { parse(highlighter, parser); });
      return;
    }

    m_activeParser = parser;
    auto rootNode = parser.parse();
    if (rootNode) {
      qDebug() << "full parse finished";
      QMetaObject::invokeMethod(highlighter, "fullParseFinished", Qt::QueuedConnection,
                                Q_ARG(RootNode, *rootNode));
    } else {
      qDebug() << "full parse canceled";
    }
  } else {
    qWarning() << "highlighter or parser is null";
  }
}

void SyntaxHighlighterThread::parse(SyntaxHighlighter* highlighter,
                                    LanguageParser parser,
                                    QList<Node> children,
                                    Region region) {
  if (highlighter) {
    if (m_activeParser) {
      if (m_activeParser->isFullParsing()) {
        qDebug() << "Start full parsing with a new text";
        m_activeParser->cancel();
        // Start full parsing with a new text
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
        QTimer::singleShot(0, this, [=] { parse(highlighter, parser); });
        return;
      } else if (m_activeParser->isParsing()) {
        qDebug() << "cancel partial parsing";
        region = m_parsingRegion ? m_parsingRegion->sum(region) : region;
        m_activeParser->cancel();
        // Start partial parsing with a new text and region
        QTimer::singleShot(0, this, [&] { parse(highlighter, parser, children, region); });
        return;
      }
    }

    m_activeParser = parser;
    m_parsingRegion = region;
    auto result = parser.parse(children, region);
    m_parsingRegion = boost::none;

    if (result) {
      qDebug() << "partial parse finished";
      QMetaObject::invokeMethod(highlighter, "partialParseFinished", Qt::QueuedConnection,
                                Q_ARG(QList<Node>, std::get<0>(*result)),
                                Q_ARG(Region, std::get<1>(*result)));
    } else {
      qDebug() << "partial parse canceled";
    }
  } else {
    qWarning() << "highlighter or parser is null";
  }
}

SyntaxHighlighterThread::SyntaxHighlighterThread() : m_thread(new QThread(this)) {
  moveToThread(m_thread);
  m_thread->start();
}

}  // namespace core
