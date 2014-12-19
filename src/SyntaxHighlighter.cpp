#include <QTextDocument>
#include <QDebug>

#include "SyntaxHighlighter.h"
#include "PListParser.h"
#include "Util.h"

// Note: QSyntaxHighlighter(QTextDocument* doc) connects contentsChange signal inside it, so pass dammy QObject first then call setDocument(doc) later to control the order of slot calls for contentsChange signal.
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc, LanguageParser *parser)
    : QSyntaxHighlighter(new QObject()), m_rootNode(parser->parse()), m_lastScopeNode(nullptr), m_parser(parser) {
  QObject::connect(doc, &QTextDocument::contentsChange, [this](int position, int charsRemoved, int charsAdded) {
    qDebug("contentsChange(pos: %d, charsRemoved: %d, charsAdded: %d)", position, charsRemoved, charsAdded);
    m_parser->setText(document()->toPlainText());
    adjust(position, charsAdded - charsRemoved);
    qDebug() << *m_rootNode;
//    m_rootNode.reset(m_parser->parse());
  });
  setDocument(doc);
}

void SyntaxHighlighter::setParser(LanguageParser *parser)
{
  if (!parser) return;

  m_rootNode.reset(parser->parse());
  m_lastScopeNode = nullptr;
}

Region SyntaxHighlighter::scopeExtent(int point) {
  updateScope(point);
  if (m_lastScopeNode) {
    return m_lastScopeNode->range;
  }
  return Region();
}

QString SyntaxHighlighter::scopeName(int point) {
  updateScope(point);
  return m_lastScopeName;
}

void SyntaxHighlighter::setTheme(const QString& themeFileName) {
  m_theme.reset(Theme::loadTheme(themeFileName));
}

void SyntaxHighlighter::adjust(int pos, int delta)
{
  qDebug("SyntaxHighlighter::adjust(pos: %d, delta: %d)", pos, delta);
  if (m_rootNode) {
    m_rootNode->adjust(pos, delta);
  }
  m_rootNode->updateChildren(Region(pos, pos + delta), m_parser.get());
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
  qDebug("highlightBlock. text: %s", qPrintable(text));
  if (!m_theme) {
    return;
  }

  int pos = currentBlock().position();
  for (int i = 0; i < text.length();) {
    updateScope(pos + i);
    if (!m_lastScopeNode) {
      qDebug("lastScopeNode is null. after updateScope(%d)", pos + i);
      return;
    }

    if (m_lastScopeNode->isLeaf()) {
      Region region = m_lastScopeNode->range;
      qDebug("%d - %d  %s", region.begin(), region.end(), qPrintable(m_lastScopeName));
      std::unique_ptr<QTextCharFormat> format = m_theme->spice(m_lastScopeName);
      if (format) {
//        qDebug("setFormat(%d, %d, %s",
//               i,
//               qMin(text.length(), region.length()),
//               qPrintable(format->foreground().color().name()));
        setFormat(i, qMin(text.length(), region.length()), *format);
      } else {
        qDebug("format not found for %s", qPrintable(m_lastScopeName));
      }
      i += region.length();
    } else {
      std::unique_ptr<QTextCharFormat> format = m_theme->spice(m_lastScopeName);
      if (format) {
//        qDebug("setFormat(%d, %d, %s",
//               i,
//               1,
//               qPrintable(format->foreground().color().name()));
        setFormat(i, 1, *format);
      } else {
        qDebug("format not found for %s", qPrintable(m_lastScopeName));
      }
      i++;
    }
  }
}

Node* SyntaxHighlighter::findScope(const Region& search, Node* node) {
  int idx = Util::binarySearch(node->children.size(), [search, node](int i) {
    return node->children[i]->range.begin() >= search.begin() || node->children[i]->range.covers(search);
  });

  while (idx < (int)node->children.size()) {
    Node* child = node->children[idx].get();
    if (child->range.begin() > search.end()) {
      break;
    }
    if (child->range.covers(search)) {
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

  if (node != m_lastScopeNode && node->range.covers(search)) {
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

  if (!m_rootNode)
    return;

  Region search(point, point + 1);
  if (m_lastScopeNode && m_lastScopeNode->range.covers(search)) {
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
