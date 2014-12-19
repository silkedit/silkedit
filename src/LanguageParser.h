#pragma once

#include <vector>
#include <unordered_map>
#include <QVector>
#include <QMap>
#include <QDebug>

#include "macros.h"
#include "Regexp.h"
#include "stlSpecialization.h"

struct Capture {
  int key;
  QString name;
};

typedef QVector<Capture> Captures;

struct Language;
class LanguageParser;
struct Node;
struct RootNode;
class Region;

struct Regex {
  std::unique_ptr<Regexp> regex;
  int lastIndex;
  int lastFound;

  Regex() : lastIndex(0), lastFound(0) {}
  explicit Regex(const QString& pattern) : regex(Regexp::compile(pattern)), lastIndex(0), lastFound(0) {}

  QVector<Region>* find(const QString& data, int pos);
  QString toString() const;

  friend QDebug operator<<(QDebug dbg, const Regex& regex) {
    dbg.nospace() << regex.toString();
    return dbg.space();
  }
};

// This struct is mutable because it has cache
struct Pattern {
  QString name;
  QString include;
  Regex match;
  Captures captures;
  Regex begin;
  Captures beginCaptures;
  Regex end;
  Captures endCaptures;
  std::unique_ptr<QVector<Pattern*>> patterns;
  Language* lang;
  QStringRef cachedStr;
  Pattern* cachedPattern;
  std::unique_ptr<QVector<Pattern*>> cachedPatterns;
  QVector<Region>* cachedRegions;
  int hits;
  int misses;

  Pattern();
  explicit Pattern(const QString& p_include);
  virtual ~Pattern() = default;

  std::pair<Pattern*, QVector<Region>*> searchInPatterns(const QString& data, int pos);
  std::pair<Pattern*, QVector<Region>*> cache(const QString& data, int pos);
  Node* createNode(const QString& data, LanguageParser* parser, const QVector<Region>& regions);
  void createCaptureNodes(LanguageParser* parser,
                          QVector<Region> regions,
                          Node* parent,
                          Captures captures);
  void tweak(Language* l);

  virtual QString toString() const;

  friend QDebug operator<<(QDebug dbg, const Pattern& pat) {
    dbg.nospace() << pat.toString();
    return dbg.space();
  }
};

class RootPattern : public Pattern {
 public:
  QString toString() const override;

  friend QDebug operator<<(QDebug dbg, const RootPattern& pat) {
    dbg.nospace() << pat.toString();
    return dbg.space();
  }
};

class LanguageProvider {
 public:
  static Language* languageFromScope(const QString& scopeName);
  static Language* languageFromExtension(const QString& ext);
  static Language* languageFromFile(const QString& path);
  static QVector<QPair<QString, QString>> langNameAndScopePairs() {
    return m_langNameAndScopePairs;
  }

  static void loadLanguages();

 private:
  static QVector<QPair<QString, QString>> m_langNameAndScopePairs;
  static QMap<QString, QString> m_scopeLangFilePathMap;
  static QMap<QString, QString> m_extensionLangFilePathMap;

  LanguageProvider() = delete;
  ~LanguageProvider() = delete;
};

// todo: check who is the owner of Language?
// Language cannot be shared (means mutable) across multiple documents because RootPattern and
// patterns in a repository have some cache and they are unique for a certain document.
struct Language {
  QVector<QString> fileTypes;
  QString firstLineMatch;
  std::unique_ptr<RootPattern> rootPattern;  // patterns
  std::unordered_map<QString, std::unique_ptr<Pattern>> repository;
  QString scopeName;

  Language() : rootPattern(nullptr) {}

  void tweak();
  QString toString() const;
  QString name();
};

class LanguageParser {
  DISABLE_COPY(LanguageParser)
 public:
  static LanguageParser* create(const QString& scope, const QString& text);

  ~LanguageParser() = default;
  DEFAULT_MOVE(LanguageParser)

  RootNode* parse();
  QVector<Node*> parse(const Region& region);
  QString getData(int start, int end);
  void setText(const QString& text) { m_text = text; }

 private:
  std::unique_ptr<Language> m_lang;
  QString m_text;

  LanguageParser(Language* lang, const QString& str);
};

class Region {
public:
  Region() : m_begin(0), m_end(0) {}
  Region(int begin, int end) {
    m_begin = qMin(begin, end);
    m_end = qMax(begin, end);
  }

  bool covers(const Region& r2);
  bool contains(int point);
  bool isEmpty() { return m_begin == m_end; }
  // Adjusts the region in place for the given position and delta
  void adjust(int pos, int delta);

  int begin() const { return m_begin; }
  void setBegin(int p);
  int end() const { return m_end; }
  void setEnd(int p);
  int length() const;

private:
  int m_begin;
  int m_end;
};

struct Node {
  DISABLE_COPY(Node)

  Region range;
  QString name;
  std::vector<std::unique_ptr<Node>> children;
  LanguageParser* parser;

  Node(LanguageParser* parser, const QString& name);
  Node(const QString& p_name, Region p_range, LanguageParser* p_p);
  virtual ~Node() = default;
  DEFAULT_MOVE(Node)

  void append(Node* child);
  Region updateRange();
  QString toString() const;
  bool isLeaf() { return children.size() == 0; }
  virtual void adjust(int pos, int delta);

  friend QDebug operator<<(QDebug dbg, const Node& node) {
    dbg.nospace() << node.toString();
    return dbg.space();
  }

 private:
  QString data() const;
  QString format(QString indent) const;
};

struct RootNode : public Node {
  RootNode(LanguageParser* parser, const QString& name);

  void adjust(int pos, int delta) override;
  QVector<Node*> nodesCovering(const Region& region);
  void updateChildren(const Region& region, LanguageParser* parser);
};
