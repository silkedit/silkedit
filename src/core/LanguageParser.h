#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <QVector>
#include <QMap>
#include <QDebug>

#include "core/macros.h"
#include "core/Regexp.h"
#include "stlSpecialization.h"
#include "core/Region.h"

namespace core {

struct Language;
class LanguageParser;
struct Node;
struct RootNode;

struct Capture {
  int key;
  QString name;
};

typedef QVector<Capture> Captures;

struct Regex {
  std::unique_ptr<Regexp> regex;
  int lastFound;

  Regex() : lastFound(0) {}
  explicit Regex(const QString& pattern) : regex(Regexp::compile(pattern)), lastFound(0) {}

  QVector<Region>* find(const QString& str, int beginPos, bool findNotEmpty);
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
  std::unique_ptr<Language> includedLanguage;
  int hits;
  int misses;

  Pattern();
  explicit Pattern(const QString& p_include);
  virtual ~Pattern() = default;

  std::pair<Pattern*, QVector<Region>*> searchInPatterns(const QString& data,
                                                         int pos,
                                                         bool findNotEmpty);
  std::pair<Pattern*, QVector<Region>*> find(const QString& data, int pos, bool findNotEmpty);
  Node* createNode(const QString& data, LanguageParser* parser, const QVector<Region>& regions);
  void createCaptureNodes(LanguageParser* parser,
                          QVector<Region> regions,
                          Node* parent,
                          Captures captures);
  void tweak(Language* l);
  void clearCache();
};

class RootPattern : public Pattern {
 public:
};

class LanguageProvider {
  DISABLE_COPY_AND_MOVE(LanguageProvider)
 public:
  static Language* defaultLanguage();
  static Language* languageFromScope(const QString& scopeName);
  static Language* languageFromExtension(const QString& ext);
  static Language* loadLanguage(const QString& path);
  static QVector<QPair<QString, QString>> scopeAndLangNamePairs() {
    return m_scopeAndLangNamePairs;
  }

 private:
  static QVector<QPair<QString, QString>> m_scopeAndLangNamePairs;
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
  Language* baseLanguage;

  Language() : rootPattern(nullptr), baseLanguage(this) {}

  void tweak();
  QString name();
  void clearCache();

  bool operator==(const Language& other) { return scopeName == other.scopeName; }
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
  void clearCache();

 private:
  std::unique_ptr<Language> m_lang;
  QString m_text;

  LanguageParser(Language* lang, const QString& str);
};

struct Node {
  DISABLE_COPY(Node)

  Region region;
  QString name;
  std::vector<std::unique_ptr<Node>> children;
  LanguageParser* parser;

  Node(LanguageParser* parser, const QString& name);
  Node(const QString& p_name, Region p_range, LanguageParser* p_p);
  virtual ~Node() = default;
  DEFAULT_MOVE(Node)

  void append(Node* child);
  Region updateRegion();
  QString toString() const;
  bool isLeaf() const { return children.size() == 0; }
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
  void updateChildren(const Region& region, LanguageParser* parser);
};

}  // namespace core
