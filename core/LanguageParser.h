#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <QVector>
#include <QMap>
#include <QDebug>
#include <QReadWriteLock>
#include <QThreadStorage>

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

  virtual boost::optional<QVector<Region>> find(
      const QString& str,
      int beginPos,
      int endPos = -1,
      QList<QStringRef> capturedStrs = QList<QStringRef>()) = 0;

  virtual QString pattern() = 0;

 protected:
  Regex() {}

  boost::optional<QVector<Region>> find(Regexp* regex,
                                        const QString& str,
                                        int beginPos,
                                        int endPos);

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

  Pattern* parent;

  // If we use QStringRef, the app crashes when entering Japanese characters in Kotoeri
  QString cachedStr;
  QAtomicPointer<Pattern> cachedResultPattern;
  QVector<Pattern*> cachedPatterns;
  boost::optional<QVector<Region>> cachedResultRegions;
  std::unique_ptr<Language> cachedIncludedLanguage;

  explicit Pattern(Language* lang, Pattern* parent = nullptr);
  virtual ~Pattern() = default;

  std::pair<Pattern*, boost::optional<QVector<Region>>> searchInPatterns(const QString& data,
                                                                         int pos, int endPos = -1);

  // Note: Don't add endPos because Pattern caches the result matched in [beginPos, end of data)
  // When you call find next time, find returns the chached result if beginPos > cached result's
  // begin pos
  std::pair<Pattern*, boost::optional<QVector<Region>>> find(const QString& data, int beginPos, int endPos = -1);
  Node createNode(const QString& data, const QVector<Region>& regions);
  void createCaptureNodes(QVector<Region> regions,
                          Node* parent,
                          Captures captures);
  void clearCache();

 private:
};

class RootPattern : public Pattern {
 public:
  explicit RootPattern(Language* lang) : Pattern(lang) {}
};

// Thread safe
class LanguageProvider {
  DISABLE_COPY_AND_MOVE(LanguageProvider)
 public:
  static Language* defaultLanguage();
  static Language* languageFromScope(const QString& scopeName);
  static Language* languageFromExtension(const QString& ext);
  static Language* loadLanguage(const QString& path);
  static QVector<QPair<QString, QString>> scopeAndLangNamePairs();

 private:
  static QVector<QPair<QString, QString>> s_scopeAndLangNamePairs;
  static QMap<QString, QString> s_scopeLangFilePathMap;
  static QMap<QString, QString> s_extensionLangFilePathMap;
  static QReadWriteLock s_lock;

  LanguageProvider() = delete;
  ~LanguageProvider() = delete;
};

// Language cannot be shared (means mutable) across multiple documents because RootPattern has some
// cache and is unique for a certain document.
struct Language {
  QVector<QString> fileTypes;
  QString firstLineMatch;
  std::unique_ptr<RootPattern> rootPattern;  // patterns
  QString scopeName;
  Language* baseLanguage;
  bool hideFromUser;

  explicit Language(QVariantMap rootMap);

  QString name();
  void clearCache();

  bool operator==(const Language& other) { return scopeName == other.scopeName; }
};

class LanguageParser {
 public:
  enum class State { Idle, FullParsing, PartialParsing, CancelRequested };

  static LanguageParser* create(const QString& scope, const QString& text);

  // Don't call default consturctor in an application side. This is for invokeMethod
  LanguageParser();
  ~LanguageParser() = default;
  DEFAULT_COPY_AND_MOVE(LanguageParser)

  boost::optional<RootNode> parse();
  boost::optional<std::tuple<QList<Node>, Region> > parse(QList<Node> children, Region region);
  QString getData(int start, int end);

  QString text();

  void setText(const QString& text);

  int beginOfLine(int pos);
  int endOfLine(int pos);

  bool isIdle();
  void setState(State state);
  bool isFullParsing();
  bool isParsing();
  void cancel();
  bool isCancelRequested();

 private:
  std::shared_ptr<Language> m_lang;
  QString m_text;
  State m_state;

  LanguageParser(std::unique_ptr<Language> lang, const QString& str);

  std::tuple<QList<Node>, Region> parse(const QString& text, QList<Node> children, Region region);
  void clearCache();
};

struct Node {
  Region region;
  QString name;
  QList<Node> children;

  // This is used in createCaptureNodes
  QList<Node*> tmpChildren;

  Node() = default;
  Node(const QString& name);
  Node(const QString& p_name, Region p_range);
  virtual ~Node() = default;
  DEFAULT_COPY_AND_MOVE(Node)

  void append(const Node& child);
  void appendTmp(Node* child);
  void moveTmpChildren();

  Region updateRegion();
  QString toString(const QString &text) const;
  bool isLeaf() const;
  virtual void adjust(int pos, int delta);

  void removeChildren(Region region);
  void addChildren(QList<Node> newNodes);
  void sortChildren();

  inline bool operator==(const Node& other) const {
    return region == other.region && name == other.name && children.size() == other.children.size();
  }

  inline bool operator!=(const Node& other) const { return !(*this == other); }

 private:
  QString data(const QString &text) const;
  QString format(QString indent, const QString &text) const;
};

struct RootNode : public Node {
  RootNode();
  RootNode(const QString& name);

  void adjust(int pos, int delta) override;
};

}  // namespace core

Q_DECLARE_METATYPE(core::Node)
Q_DECLARE_METATYPE(core::RootNode)
Q_DECLARE_METATYPE(core::LanguageParser)
