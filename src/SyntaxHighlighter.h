#pragma once

#include <memory>
#include <QSyntaxHighlighter>

#include "macros.h"
#include "LanguageParser.h"
#include "Theme.h"

// The SyntaxHighlighter interface is responsible for
// identifying the extent and name of code scopes given
// a position in the code buffer this specific SyntaxHighlighter
// is responsible for.
//
// It's expected that the syntax highlighter monkey patches its existing
// scope data rather than performing a full reparse when the underlying
// buffer changes.
//
// This is because a full reparse, for which the Parser interface is responsible,
// will be going on in parallel in a separate thread and the "monkey patch"
// will allow some accuracy in the meantime until the Parse operation has finished.
class SyntaxHighlighter : public QSyntaxHighlighter {
  DISABLE_COPY(SyntaxHighlighter)

 public:
  ~SyntaxHighlighter() = default;
  DEFAULT_MOVE(SyntaxHighlighter)

  static SyntaxHighlighter* create(QTextDocument* doc, LanguageParser* parser);

  void setParser(LanguageParser* parser);

  // Returns the Region of the inner most Scope extent which contains "point".
  //
  // This method can be called a lot by plugins, and should therefore be as
  // fast as possible.
  Region scopeExtent(int point);

  // Returns the full concatenated nested scope name of the scope(s) containing "point".
  //
  // This method can be called a lot by plugins, and should therefore be as
  // fast as possible.
  QString scopeName(int point);

  void setTheme(const QString& themeFileName);

 protected:
  void highlightBlock(const QString& text) override;

 private:
  std::unique_ptr<Node> m_rootNode;
  Node* m_lastScopeNode;
  QByteArray m_lastScopeBuf;
  QString m_lastScopeName;
  std::unique_ptr<Theme> m_theme;

  SyntaxHighlighter(QTextDocument* doc, Node* root);

  // Given a text region, returns the innermost node covering that region.
  // Side-effects: Writes to m_lastScopeBuf...
  Node* findScope(const Region& search, Node* node);

  // Caches the full concatenated nested scope name and the innermost node that covers "point".
  void updateScope(int point);
};
