﻿#include <QTextDocument>
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
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc, LanguageParser* parser)
    : QSyntaxHighlighter(new QObject()),
      m_rootNode(parser->parse()),
      m_lastScopeNode(nullptr),
      m_parser(parser) {
  // this connect causes crash when opening a file and editing it then closing it without save.
  //  auto conn = connect(doc, &QTextDocument::contentsChange, this,
  // &SyntaxHighlighter::updateNode);
  connect(doc, SIGNAL(contentsChange(int, int, int)), this, SLOT(updateNode(int, int, int)));
  connect(&Config::singleton(), SIGNAL(themeChanged(Theme*)), this, SLOT(changeTheme(Theme*)));
  connect(&Config::singleton(), SIGNAL(fontChanged(QFont)), this, SLOT(changeFont(QFont)));

  setDocument(doc);
  setParent(doc);
  changeTheme(Config::singleton().theme());
  changeFont(Config::singleton().font());
}

SyntaxHighlighter::~SyntaxHighlighter() {
  qDebug("~SyntaxHighlighter");
}

void SyntaxHighlighter::setParser(LanguageParser* parser) {
  if (!parser)
    return;

  m_parser.reset(parser);
  m_rootNode.reset(parser->parse());
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
  //  qDebug("SyntaxHighlighter::adjust(pos: %d, delta: %d)", pos, delta);
  if (m_rootNode) {
    m_rootNode->adjust(pos, delta);
  }

  QTextBlock beginBlock = document()->findBlock(pos);
  if (!beginBlock.isValid()) {
    qDebug("beginBlock is invalid");
    return;
  }

  QTextBlock endBlock = document()->findBlock(pos + delta);
  if (!endBlock.isValid()) {
    qDebug("endBlock is invalid");
    return;
  }

  m_rootNode->updateChildren(Region(beginBlock.position(), endBlock.position() + endBlock.length()),
                             m_parser.get());
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
    // INFO: When pasting a text in an empty document, charsRemoved becomes 1 because of this bug.
    // We added -1 as a workaround.
    // https://bugreports.qt.io/browse/QTBUG-3495
    adjust(position + charsRemoved - 1, charsAdded - charsRemoved);
  }
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
  if (!m_theme) {
    qDebug("theme is null");
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

}  // namespace core