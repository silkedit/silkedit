#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <QVector>
#include <QMap>
#include <QDebug>

#include "macros.h"
#include "Regexp.h"
#include "stlSpecialization.h"
#include "Region.h"

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
  static Regex* create(const QString& pattern);

  virtual ~Regex() = default;

  virtual boost::optional<QVector<Region>>
  find(const QString& str, int beginPos, int endPos = -1, QList<QStringRef> capturedStrs = QList<QStringRef>()) = 0;

  virtual QString pattern() = 0;

 protected:
  Regex() {}

  boost::optional<QVector<Region>> find(Regexp* regex, const QString& str, int beginPos, int endPos);

 private:
  friend class LanguageParserTest;

  static bool hasBackReference(const QString& str);
};

// regex without back reference
struct FixedRegex : public Regex {
  std::unique_ptr<Regexp> regex;

  explicit FixedRegex(const QString& pattern);

  QString pattern() override;

  boost::optional<QVector<Region>> find(
      const QString& str,
      int beginPos,
      int endPos,
      QList<QStringRef> capturedStrs = QList<QStringRef>()) override;
};

// regex with back reference. e.g. \s*\2$\n?
// end pattern can have back references captured in begin regex
struct RegexWithBackReference : public Regex {
  QString patternStr;

  explicit RegexWithBackReference(const QString& pattern) : Regex(), patternStr(pattern) {}

  QString pattern() override { return patternStr; }

  boost::optional<QVector<Region>> find(
      const QString& str,
      int beginPos,
      int endPos,
      QList<QStringRef> capturedStrs = QList<QStringRef>()) override;
};

// This struct is mutable because it has cache
struct Pattern {
  // name could be empty
  // e.g. root patterns in Property List (XML)
  QString name;

  QString contentName;
  QString include;
  std::unique_ptr<Regex> match;
  Captures captures;
  std::unique_ptr<Regex> begin;
  Captures beginCaptures;
  std::unique_ptr<Regex> end;
  Captures endCaptures;
  std::unique_ptr<QVector<Pattern*>> patterns;
  std::unordered_map<QString, std::unique_ptr<Pattern>> repository;
  Language* lang;
  QStringRef cachedStr;
  Pattern* cachedResultPattern;
  std::unique_ptr<QVector<Pattern*>> cachedPatterns;
  boost::optional<QVector<Region>> cachedResultRegions;
  Pattern* parent;
  std::unique_ptr<Language> includedLanguage;
  int hits;
  int misses;

  explicit Pattern(Pattern* parent = nullptr);
  virtual ~Pattern() = default;

  std::pair<Pattern*, boost::optional<QVector<Region>>> searchInPatterns(const QString& data,
                                                                         int pos);

  // Note: Don't add endPos because Pattern caches the result matched in [beginPos, end of data)
  // When you call find next time, find returns the chached result if beginPos > cached result's begin pos
  std::pair<Pattern*, boost::optional<QVector<Region>>> find(const QString& data, int beginPos);
  std::unique_ptr<Node> createNode(const QString& data,
                                   LanguageParser* parser,
                                   const QVector<Region>& regions);
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
// Language cannot be shared (means mutable) across multiple documents because RootPattern has some
// cache and is unique for a certain document.
struct Language {
  QVector<QString> fileTypes;
  QString firstLineMatch;
  std::unique_ptr<RootPattern> rootPattern;  // patterns
  QString scopeName;
  Language* baseLanguage;
  bool hideFromUser;

  Language() : rootPattern(nullptr), baseLanguage(this), hideFromUser(false) {}

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

  std::unique_ptr<RootNode> parse();
  std::vector<std::unique_ptr<Node>> parse(const Region& region);
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

  void append(std::unique_ptr<Node> child);
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
