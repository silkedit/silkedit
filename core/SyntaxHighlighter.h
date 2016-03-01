#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <QSyntaxHighlighter>
#include <QThread>

#include "macros.h"
#include "LanguageParser.h"
#include "Singleton.h"
#include "Region.h"

namespace core {

class Theme;
class SyntaxHighlighter;

class SyntaxHighlighterThread : public QObject, public Singleton<SyntaxHighlighterThread> {
  Q_OBJECT
 public:
  ~SyntaxHighlighterThread() = default;
  void quit();

 public slots:
  void parse(SyntaxHighlighter* highlighter, LanguageParser parser);
  void parse(SyntaxHighlighter* highlighter,
             LanguageParser parser,
             QList<Node> children, Region region);

 private:
  QThread* m_thread;
  boost::optional<LanguageParser> m_activeParser;
  boost::optional<Region> m_parsingRegion;

  friend class Singleton<SyntaxHighlighterThread>;

  SyntaxHighlighterThread();
};

class SyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
  DISABLE_COPY(SyntaxHighlighter)

 public:
  SyntaxHighlighter(QTextDocument* doc,
                    std::unique_ptr<LanguageParser> parser,
                    Theme* theme,
                    QFont font);
  ~SyntaxHighlighter();
  DEFAULT_MOVE(SyntaxHighlighter)

  // accessor
  RootNode rootNode() { return *m_rootNode; }

  void setParser(LanguageParser parser);

  // Returns the Region of the inner most Scope extent which contains "point".
  Region scopeExtent(int point);

  // Returns the full concatenated nested scope name of the scope(s) containing "point".
  QString scopeName(int point);

  QString scopeTree();

  /**
   * @brief adjust
   * @param pos position before modification happened
   * @param delta
   */
  void adjust(int pos, int delta);

  QString asHtml();

 signals:
  void parseFinished();

 public slots:
  void updateNode(int position, int charsRemoved, int charsAdded);
  void fullParseFinished(RootNode node);
  void partialParseFinished(QList<Node> newNodes, Region region);

 protected:
  void highlightBlock(const QString& text) override;

 private:
  boost::optional<RootNode> m_rootNode;
  boost::optional<Node> m_lastScopeNode;
  QByteArray m_lastScopeBuf;
  QString m_lastScopeName;
  boost::optional<LanguageParser> m_parser;
  Theme* m_theme;

  // Given a text region, returns the innermost node covering that region.
  // Side-effects: Writes to m_lastScopeBuf...
  boost::optional<Node> findScope(const Region& search, const Node& node);

  // Caches the full concatenated nested scope name and the innermost node that covers "point".
  void updateScope(int point);

 private slots:
  void changeTheme(Theme* theme);
  void changeFont(const QFont& font);
};

}  // namespace core

Q_DECLARE_METATYPE(core::SyntaxHighlighter*)
