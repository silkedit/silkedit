#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QDebug>

#include "SyntaxHighlighter.h"
#include "PListParser.h"
#include "Util.h"
#include "Config.h"
#include "LanguageParser.h"
#include "Theme.h"

namespace core {

// fixme: memory leak of new QObject();
// Note: QSyntaxHighlighter(QTextDocument* doc) connects contentsChange signal inside it, so pass
// dammy QObject first then call setDocument(doc) later to control the order of slot calls for
// contentsChange signal.
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc,
                                     std::unique_ptr<LanguageParser> parser,
                                     Theme* theme,
                                     QFont font)
    : QSyntaxHighlighter(new QObject()),
      m_rootNode(parser->parse()),
      m_lastScopeNode(nullptr),
      m_parser(std::move(parser)),
      m_theme(theme),
      m_font(font) {
  setDocument(doc);

  /*
   setDocument creates connection below which calls highlightBlock in _q_reformatBlocks by
   contentsChange signal, but we want to override this behavior, so disconnect it.
   connect(d->doc, SIGNAL(contentsChange(int,int,int)),
     this, SLOT(_q_reformatBlocks(int,int,int)));
   */
  disconnect(doc, SIGNAL(contentsChange(int, int, int)), this,
             SLOT(_q_reformatBlocks(int, int, int)));

  setParent(doc);

  // this connect causes crash when opening a file and editing it then closing it without save.
  //  auto conn = connect(doc, &QTextDocument::contentsChange, this,
  // &SyntaxHighlighter::updateNode);
  connect(doc, SIGNAL(contentsChange(int, int, int)), this, SLOT(updateNode(int, int, int)));
  connect(&Config::singleton(), &Config::themeChanged, this, &SyntaxHighlighter::changeTheme);
  connect(&Config::singleton(), &Config::fontChanged, this, &SyntaxHighlighter::changeFont);

  rehighlight();
}

SyntaxHighlighter::~SyntaxHighlighter() {
  qDebug("~SyntaxHighlighter");
}

void SyntaxHighlighter::setParser(LanguageParser* parser) {
  if (!parser)
    return;

  m_parser.reset(parser);
  m_rootNode = std::move(parser->parse());
  m_lastScopeNode = nullptr;
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

QString SyntaxHighlighter::scopeTree() const {
  return m_rootNode ? m_rootNode->toString() : "";
}

void SyntaxHighlighter::adjust(int pos, int delta) {
  qDebug() << "adjust(pos:" << pos << ", delta:" << delta << ")";
  if (m_rootNode) {
    m_rootNode->adjust(pos, delta);
  }

  int beginPos, endPos;
  if (delta > 0) {
    beginPos = pos;
    endPos = pos + delta;
  } else {
    beginPos = pos + delta;
    endPos = pos;
  }

//   We need to extend affectedRegion to the region from the beginning of the line at beginPos to
//   the end of the line at endPos to support look ahead and behind regex.
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
//   In this case, [0-32) is not updated without expansion because [0-32) doesn't intersect [32-33).
//   But we need to update [0-32) because its end pattern has /(?=/ and end pattern should match with
//   '/' at pos 36

  QTextBlock beginBlock = document()->findBlock(beginPos);
  if (!beginBlock.isValid()) {
    qWarning("beginBlock is invalid");
    return;
  }

  QTextBlock endBlock = document()->findBlock(endPos);
  if (!endBlock.isValid()) {
    qWarning("endBlock is invalid");
    return;
  }

  const auto& affectedRegion = m_rootNode->updateChildren(Region(beginBlock.position(), endBlock.position() + endBlock.length()),
                             m_parser.get());

//  qDebug().noquote() << "affectedRegion:" << affectedRegion;

  QTextBlock affectedBlock = document()->findBlock(affectedRegion.begin());
  while (affectedBlock.isValid() && affectedBlock.position() < affectedRegion.end()) {
    rehighlightBlock(affectedBlock);
    affectedBlock = affectedBlock.next();
  }

  m_lastScopeNode = nullptr;
  m_lastScopeBuf.clear();
  m_lastScopeName = "";
}

void SyntaxHighlighter::updateNode(int position, int charsRemoved, int charsAdded) {
  qDebug("contentsChange(pos: %d, charsRemoved: %d, charsAdded: %d)", position, charsRemoved,
         charsAdded);
  if (document()) {
    m_parser->setText(document()->toPlainText());
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
    adjust(position + charsRemoved, charsAdded - charsRemoved);
  }
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
      qDebug("lastScopeNode is null. after updateScope(%d)", posInDoc + posInText);
      return;
    }

    std::shared_ptr<QTextCharFormat> format = m_theme->getFormat(m_lastScopeName);
    if (format) {
      // This font must match the Document default font
      format->setFont(m_font);
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
    }
  }
}

Node* SyntaxHighlighter::findScope(const Region& search, Node* node) {
  if (!node) return nullptr;

  size_t idx = Util::binarySearch(node->children.size(), [search, node](size_t i) {
    return node->children[i]->region.begin() >= search.begin() ||
           node->children[i]->region.fullyCovers(search);
  });

  while (idx < node->children.size()) {
    Node* child = node->children[idx].get();
    if (child->region.begin() > search.end()) {
      break;
    }
    if (child->region.fullyCovers(search)) {
      if (!node->name.isEmpty() && node != m_lastScopeNode) {
        if (m_lastScopeBuf.length() > 0) {
          m_lastScopeBuf.append(' ');
        }
        m_lastScopeBuf.append(node->name);
      }
      return findScope(search, node->children[idx].get());
    }
    idx++;
  }

  if (node != m_lastScopeNode && node->region.fullyCovers(search)) {
    if (!node->name.isEmpty()) {
      if (m_lastScopeBuf.length() > 0) {
        m_lastScopeBuf.append(' ');
      }
      m_lastScopeBuf.append(node->name);
    }
    return node;
  }

  return nullptr;
}

void SyntaxHighlighter::updateScope(int point) {
  //  qDebug("updateScope(point: %d)", point);

  if (!m_rootNode) {
    qDebug("root node is null");
    return;
  }

  Region search(point, point + 1);
  if (m_lastScopeNode && m_lastScopeNode->region.fullyCovers(search)) {
    if (m_lastScopeNode->children.size() != 0) {
      Node* no = findScope(search, m_lastScopeNode);
      if (no && no != m_lastScopeNode) {
        m_lastScopeNode = no;
        m_lastScopeName = QString(m_lastScopeBuf);
      }
    }
    return;
  }

  m_lastScopeNode = nullptr;
  m_lastScopeBuf.clear();
  m_lastScopeNode = findScope(search, m_rootNode.get());
  m_lastScopeName = QString(m_lastScopeBuf);
}

void SyntaxHighlighter::changeTheme(Theme* theme) {
  m_theme = theme;
  rehighlight();
}

void SyntaxHighlighter::changeFont(const QFont& font) {
  m_font = font;
  rehighlight();
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
  for (QTextBlock current = start; current.isValid() and current not_eq end;
       current = current.next()) {
    const QTextLayout* layout(current.layout());

    foreach (const QTextLayout::FormatRange& range, layout->additionalFormats()) {
      const int start = current.position() + range.start - selectionStart;
      const int end = start + range.length;
      if (end <= 0 or start >= endOfDocument)
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

}  // namespace core
