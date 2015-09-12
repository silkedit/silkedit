#pragma once

#include <memory>
#include <QSyntaxHighlighter>

#include "core/macros.h"

namespace core {
class LanguageParser;
struct Node;
struct RootNode;
class Region;
class Theme;
}

class SyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
  DISABLE_COPY(SyntaxHighlighter)

 public:
  SyntaxHighlighter(QTextDocument* doc, core::LanguageParser* parser);
  ~SyntaxHighlighter();
  DEFAULT_MOVE(SyntaxHighlighter)

  // accessor
  core::RootNode* rootNode() { return m_rootNode.get(); }

  void setParser(core::LanguageParser* parser);

  // Returns the Region of the inner most Scope extent which contains "point".
  core::Region scopeExtent(int point);

  // Returns the full concatenated nested scope name of the scope(s) containing "point".
  QString scopeName(int point);

  QString scopeTree() const;

  /**
   * @brief adjust
   * @param pos position before modification happened
   * @param delta
   */
  void adjust(int pos, int delta);

 public slots:
  void updateNode(int position, int charsRemoved, int charsAdded);

 protected:
  void highlightBlock(const QString& text) override;

 private:
  std::unique_ptr<core::RootNode> m_rootNode;
  core::Node* m_lastScopeNode;
  QByteArray m_lastScopeBuf;
  QString m_lastScopeName;
  core::Theme* m_theme;
  std::unique_ptr<core::LanguageParser> m_parser;
  QFont m_font;

  // Given a text region, returns the innermost node covering that region.
  // Side-effects: Writes to m_lastScopeBuf...
  core::Node* findScope(const core::Region& search, core::Node* node);

  // Caches the full concatenated nested scope name and the innermost node that covers "point".
  void updateScope(int point);

 private slots:
  void changeTheme(core::Theme* theme);
  void changeFont(const QFont& font);
};
