#pragma once

#include <QVector>
#include <QMap>
#include <QDebug>

#include "macros.h"
#include "Regexp.h"

class TmLanguage {
  DISABLE_COPY(TmLanguage)

 public:
  TmLanguage();
  ~TmLanguage() = default;
  DEFAULT_MOVE(TmLanguage)
  // func (lp *LanguageParser) Parse() (*parser.Node, error) {

 private:
};

struct Capture {
  int key;
  QString name;
};

typedef QVector<Capture> Captures;
typedef QVector<int> MatchObject;

struct Regex {
  Regexp* re;
  int lastIndex;
  int lastFound;

  Regex() : re(nullptr), lastIndex(0), lastFound(0) {}
  Regex(const QString& pattern) : re(Regexp::compile(pattern)), lastIndex(0), lastFound(0) {}

  MatchObject* find(const QString& data, int pos);
  // func (r *Regex) Find(data string, pos int) MatchObject {
  QString toString() const;

  friend QDebug operator<<(QDebug dbg, const Regex& regex) {
    dbg.nospace() << regex.toString();
    return dbg.space();
  }
};

struct Named {
  QString name;
};

class DataSource {
 public:
  virtual ~DataSource() = default;
  virtual QString getData(int start, int end) = 0;
};

class Language;
class Node;

class Pattern {
 public:
  QString name;
  QString include;
  Regex match;
  Captures captures;
  Regex begin;
  Captures beginCaptures;
  Regex end;
  Captures endCaptures;
  QVector<Pattern*>* patterns;
  Language* owner;
  QString cachedData;
  Pattern* cachedPat;
  QVector<Pattern*>* cachedPatterns;
  MatchObject* cachedMatch;
  int hits;
  int misses;

  Pattern();
  Pattern(const QString& p_include);
  virtual ~Pattern() = default;

  std::pair<Pattern*, MatchObject*> firstMatch(const QString& data, int pos);
  // func (p *Pattern) FirstMatch(data string, pos int) (pat *Pattern, ret MatchObject) {

  std::pair<Pattern*, MatchObject*> cache(const QString& data, int pos);
  //  func (p *Pattern) Cache(data string, pos int) (pat *Pattern, ret MatchObject) {

  Node* createNode(const QString& data, int pos, DataSource* d, MatchObject* mo);
  //  func (p *Pattern) CreateNode(data string, pos int, d parser.DataSource, mo MatchObject) (ret
  //  *parser.Node) {

  void createCaptureNodes(const QString& data,
                          int pos,
                          DataSource* d,
                          MatchObject* mo,
                          Node* parent,
                          Captures capt);
  // func (p *Pattern) CreateCaptureNodes(data string, pos int, d parser.DataSource, mo MatchObject,
  // parent *parser.Node, capt Captures) {

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
  static QVector<QPair<QString, QString>> langNameAndScopePairs() { return m_langNameAndScopePairs; }

  static void loadLanguages();

private:
  static QVector<QPair<QString, QString>> m_langNameAndScopePairs;
  static QMap<QString, QString> scopeLangFilePathMap;
  static QMap<QString, QString> extensionLangFilePathMap;

  LanguageProvider();
};

// Language cannot be shared (means mutable) across multiple documents because RootPattern and patterns in a repository have some cache and they are unique for a certain document.
class Language {
 public:
  QVector<QString> fileTypes;
  QString firstLineMatch;
  RootPattern* rootPattern;  // patterns
  QMap<QString, Pattern*> repository;
  QString scopeName;

  Language() : rootPattern(nullptr) {}

  void tweak();
  QString toString() const;
  QString name();
};

class LanguageParser : public DataSource {
 public:
  Language* l;
  QVector<QChar> data;

  static LanguageParser* create(const QString& scope, const QString& data);

  Node* parse();
  QString getData(int start, int end) override;

 private:
  LanguageParser(Language* lang);
  void patch(QVector<int> lut, Node* node);
  //  func (lp *LanguageParser) patch(lut []int, node *parser.Node) {
};

struct Region {
  int a;
  int b;

  Region() : a(0), b(0) {}
  Region(int p_a, int p_b) {
    a = p_a;
    b = p_b;
  }

  bool covers(const Region& r2);
  // func (r Region) Covers(r2 Region) bool {
  bool contains(int point);
  // func (r Region) Contains(point int) bool {

  // Returns the starting point of the region,
  // that would be whichever of A and B
  // is the minimal value.
  //  func (r Region) Begin() int {
  int begin() const;

  // Returns the ending point of the region,
  // that would be whichever of A and B
  // is the maximum value.
  //  func (r Region) End() int {
  int end() const;

  int length() const;
};

class Node {
 public:
  Region range;
  QString name;
  QVector<Node*> children;
  DataSource* p;

  Node(DataSource* p, const QString& name);
  Node(const QString& p_name, Region p_range, DataSource* p_p);

  void append(Node* child);
  //  func (n *Node) Append(child *Node) {
  Region updateRange();
  //  func (n *Node) UpdateRange() text.Region {

  QString toString() const;
  bool isLeaf() { return children.size() == 0; }

  friend QDebug operator<<(QDebug dbg, const Node& node) {
    dbg.nospace() << node.toString();
    return dbg.space();
  }

private:
  QString data() const;
  QString format(QString indent) const;
};
