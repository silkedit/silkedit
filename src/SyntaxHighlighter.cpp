#include <QTextDocument>
#include <QDebug>

#include "SyntaxHighlighter.h"
#include "PListParser.h"
#include "Util.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc, Node *root) : QSyntaxHighlighter(doc), m_rootNode(root), m_lastScopeNode(nullptr) {

//  QString fileName = "packages/Solarized (Light).tmTheme";
//  QFile file(fileName);
//  if (!file.open(QIODevice::ReadOnly)) {
//    qWarning("unable to open a file");
//    return;
//  }

//  QVariant root = PListParser::parsePList(&file);
//  if (!root.canConvert<QVariantMap>()) {
//    qDebug("root is not dict");
//    return;
//  }

//  QVariantMap rootMap = root.toMap();
//  QString name = rootMap["name"].toString();
  //  qDebug() << "Theme name:" << name;
}

SyntaxHighlighter *SyntaxHighlighter::create(QTextDocument *doc, LanguageParser *parser)
{
  Node* rootNode = parser->parse();
  if (rootNode) {
    return new SyntaxHighlighter(doc, rootNode);
  } else {
    return nullptr;
  }

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

void SyntaxHighlighter::highlightBlock(const QString& text) {
}

Node* SyntaxHighlighter::findScope(const Region& search, Node* node) {
  int idx = Util::binarySearch(node->children.length(), [search, node](int i) {
    return node->children[i]->range.a >= search.a || node->children[i]->range.covers(search);
  });

  while (idx < node->children.length()) {
    Node* c = node->children[idx];
    if (c->range.a > search.b) {
      break;
    }
    if (c->range.covers(search)) {
      if (!node->name.isEmpty() && node != m_lastScopeNode) {
        if (m_lastScopeBuf.length() > 0) {
          m_lastScopeBuf.append(' ');
        }
        m_lastScopeBuf.append(node->name);
      }
      return findScope(search, node->children[idx]);
    }
    idx++;
  }

  if (node != m_lastScopeNode && node->range.covers(search) && !node->name.isEmpty()) {
    if (m_lastScopeBuf.length() > 0) {
      m_lastScopeBuf.append(' ');
    }
    m_lastScopeBuf.append(node->name);
    return node;
  }

  return nullptr;
}

void SyntaxHighlighter::updateScope(int point) {
  if (!m_rootNode)
    return;

  Region search(point, point + 1);
  if (m_lastScopeNode && m_lastScopeNode->range.covers(search)) {
    if (m_lastScopeNode->children.length() != 0) {
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
  m_lastScopeNode = findScope(search, m_rootNode);
  m_lastScopeName = QString(m_lastScopeBuf);
}
