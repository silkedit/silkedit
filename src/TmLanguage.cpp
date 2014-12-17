#include <QStringList>
#include <QStringBuilder>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QDir>

#include "TmLanguage.h"
#include "PListParser.h"

namespace {

const int maxiter = 10000;
QMap<QString, bool> failed;

// Clamps v to be in the range of _min and _max
int clamp(int min, int max, int v) {
  return qMax(min, qMin(max, v));
}

void fix(MatchObject* m, int add) {
  for (int i = 0; i < m->length(); i++) {
    if ((*m)[i] != -1) {
      (*m)[i] += add;
    }
  }
}

Captures toCaptures(QVariantMap map) {
  Captures captures;
  bool ok;
  QMapIterator<QString, QVariant> i(map);
  const QString nameStr = "name";
  while (i.hasNext()) {
    i.next();
    int key = i.key().toInt(&ok);
    if (ok && i.value().canConvert<QVariantMap>()) {
      QVariantMap subMap = i.value().toMap();
      if (subMap.contains(nameStr)) {
        QString name = subMap.value(nameStr).toString();
        captures.append(Capture{key, name});
      }
    }
  }

  return captures;
}

Pattern* toPattern(QVariantMap map);

QVector<Pattern*>* toPatterns(QVariant patternsVar) {
  if (patternsVar.canConvert<QVariantList>()) {
    QVector<Pattern*>* patterns = new QVector<Pattern*>(0);
    QSequentialIterable iterable = patternsVar.value<QSequentialIterable>();
    foreach (const QVariant& v, iterable) {
      if (v.canConvert<QVariantMap>()) {
        patterns->append(toPattern(v.toMap()));
      }
    }
    return patterns;
  }
  return nullptr;
}

void toPattern(QVariantMap map, Pattern* pat) {
  // include
  const QString include = "include";
  if (map.contains(include)) {
    pat->include = map.value(include).toString();
  }

  // match
  const QString match = "match";
  if (map.contains(match)) {
    pat->match = Regex(map.value(match).toString());
  }

  // name
  const QString name = "name";
  if (map.contains(name)) {
    pat->name = map.value(name).toString();
  }

  // begin
  const QString begin = "begin";
  if (map.contains(begin)) {
    pat->begin = Regex(map.value(begin).toString());
  }

  // beginCaptures
  const QString beginCaptures = "beginCaptures";
  if (map.contains(beginCaptures)) {
    QVariant beginCapturesVar = map.value(beginCaptures);
    if (beginCapturesVar.canConvert<QVariantMap>()) {
      pat->beginCaptures = toCaptures(beginCapturesVar.toMap());
    }
  }

  // end
  const QString end = "end";
  if (map.contains(end)) {
    pat->end = Regex(map.value(end).toString());
  }

  // endCaptures
  const QString endCaptures = "endCaptures";
  if (map.contains(endCaptures)) {
    QVariant endCapturesVar = map.value(endCaptures);
    if (endCapturesVar.canConvert<QVariantMap>()) {
      pat->endCaptures = toCaptures(endCapturesVar.toMap());
    }
  }

  // captures
  const QString captures = "captures";
  if (map.contains(captures)) {
    QVariant capturesVar = map.value(captures);
    if (capturesVar.canConvert<QVariantMap>()) {
      pat->captures = toCaptures(capturesVar.toMap());
    }
  }

  // patterns
  const QString patternsStr = "patterns";
  if (map.contains(patternsStr)) {
    QVariant patternsVar = map.value(patternsStr);
    pat->patterns = toPatterns(patternsVar);
  }
}

RootPattern* toRootPattern(QVariantMap map) {
  RootPattern* pat = new RootPattern();
  toPattern(map, pat);
  return pat;
}

Pattern* toPattern(QVariantMap map) {
  Pattern* pat = new Pattern();
  toPattern(map, pat);
  return pat;
}
}

LanguageParser* LanguageParser::create(const QString& scopeName, const QString& data) {
  if (Language* lang = LanguageProvider::languageFromScope(scopeName)) {
    LanguageParser* parser = new LanguageParser(lang);
    for (int i = 0; i < data.length(); i++) {
      parser->data.append(data.at(i));
    }
    return parser;
  } else {
    return nullptr;
  }
}

Node* LanguageParser::parse() {
  QStringList words;
  foreach (const QChar& ch, data) { words << ch; }
  QString sdata;
  for (auto it = words.constBegin(); it != words.constEnd(); ++it) {
    sdata = sdata % *it;
  }
  //  qDebug() << sdata;

  Node* rn = new Node(this, l->scopeName);
  int iter = maxiter;
  for (int i = 0; i < sdata.length() && iter > 0; iter--) {
    auto pair = l->rootPattern->cache(sdata, i);
    Pattern* pat = pair.first;
    MatchObject* ret = pair.second;
    //    if (pat && ret) {
    //      qDebug() << "pat:" << *pat << "ret:" << *ret;
    //    }
    int nl = sdata.indexOf(QRegularExpression("\n|\r"), i);
    if (nl != -1) {
      nl += i;
    }
    if (!ret) {
      break;
    } else if (nl > 0 && nl <= (*ret)[0]) {
      i = nl;
      while (i < sdata.length() && (sdata[i] == '\n' || sdata[i] == '\r')) {
        i++;
      }
    } else {
      Node* n = pat->createNode(sdata, i, this, ret);
      rn->append(n);
      i = n->range.b;
    }
  }
  rn->updateRange();
  if (sdata.length() != 0) {
    QVector<int> lut(sdata.length() + 1);
    int j = 0;
    for (int i = 0; i < sdata.length(); i++) {
      lut[i] = j;
      j++;
    }
    lut[sdata.length()] = data.length();
    patch(lut, rn);
  }
  if (iter == 0) {
    throw "reached maximum number of iterations";
  }
  return rn;
}

QString LanguageParser::getData(int a, int b) {
  a = clamp(0, data.length(), a);
  b = clamp(0, data.length(), b);
  QVector<QChar> charVector = data.mid(a, b - a);
  QString ret;
  foreach (const QChar& ch, charVector) { ret = ret % ch; }
  return ret;
}

LanguageParser::LanguageParser(Language* lang) : l(lang) {
}

void LanguageParser::patch(QVector<int> lut, Node* node) {
  node->range.a = lut[node->range.a];
  node->range.b = lut[node->range.b];
  foreach (Node* child, node->children) { patch(lut, child); }
}

Node::Node(DataSource* p_p, const QString& p_name) {
  p = p_p;
  name = p_name;
}

Node::Node(const QString& p_name, Region p_range, DataSource* p_p) {
  name = p_name;
  range = p_range;
  p = p_p;
}

void Node::append(Node* child) {
  //  n.Children = append(n.Children, child)
  children.append(child);
}

Region Node::updateRange() {
  foreach (Node* child, children) {
    Region curr = child->updateRange();
    if (curr.begin() < range.a) {
      range.a = curr.begin();
    }
    if (curr.end() > range.b) {
      range.b = curr.end();
    }
  }
  return range;
}

QString Node::toString() const {
  return format("");
}

QString Node::format(QString indent) const {
  if (children.length() == 0) {
    return indent +
           QString("%1-%2: \"%3\" - Data: \"%4\"\n")
               .arg(range.begin())
               .arg(range.end())
               .arg(name)
               .arg(data());
  }
  QString ret;
  ret = ret % indent % QString("%1-%2: \"%3\"\n").arg(range.begin()).arg(range.end()).arg(name);
  indent += "\t";
  foreach (Node* child, children) { ret = ret % child->format(indent); }
  return ret;
}

QString Node::data() const {
  return p->getData(range.begin(), range.end());
}

Pattern::Pattern() : Pattern("") {
}

Pattern::Pattern(const QString& p_include)
    : include(p_include),
      patterns(nullptr),
      owner(nullptr),
      cachedData(""),
      cachedPat(nullptr),
      cachedPatterns(nullptr),
      cachedMatch(nullptr) {
}

std::pair<Pattern*, MatchObject*> Pattern::firstMatch(const QString& data, int pos) {
  //  qDebug("firstMatch. pos: %d", pos);
  int startIdx = -1;
  Pattern* pat = nullptr;
  MatchObject* ret = nullptr;
  for (int i = 0; i < cachedPatterns->length();) {
    auto pair = (*cachedPatterns)[i]->cache(data, pos);
    Pattern* ip = pair.first;
    MatchObject* im = pair.second;
    if (im) {
      if (startIdx < 0 || startIdx > (*im)[0]) {
        startIdx = (*im)[0];
        pat = ip;
        ret = im;
        // This match is right at the start, we're not going to find a better pattern than this, so
        // stop the search
        if ((*im)[0] == pos) {
          break;
        }
      }
      i++;
    } else {
      // If it wasn't found now, it'll never be found, so the pattern can be popped from the cache
      cachedPatterns->removeAt(i);
    }
  }

  return std::make_pair(pat, ret);
}

std::pair<Pattern*, MatchObject*> Pattern::cache(const QString& data, int pos) {
  //  qDebug("cache. pos: %d. data.size: %d", pos, data.size());
  if (!cachedData.isEmpty() && cachedData == data) {
    if (!cachedMatch) {
      //      qDebug("cachedMatch is null");
      return std::make_pair(nullptr, nullptr);
    }
    if ((*cachedMatch)[0] >= pos && cachedPat->cachedMatch) {
      //      qDebug("hits++");
      hits++;
      return std::make_pair(cachedPat, cachedMatch);
    }
  } else {
    //    qDebug("cachedPatterns = nullptr");
    cachedPatterns = nullptr;
  }
  if (!cachedPatterns) {
    cachedPatterns = new QVector<Pattern*>(patterns ? patterns->size() : 0);
    //    qDebug("copying patterns to cachedPatterns. cachedPatterns.size: %d",
    //    cachedPatterns->size());
    for (int i = 0; i < cachedPatterns->size(); i++) {
      (*cachedPatterns)[i] = (*patterns)[i];
    }
    if (patterns) {
      Q_ASSERT(cachedPatterns->size() == patterns->size());
    } else {
      Q_ASSERT(cachedPatterns->size() == 0);
    }
  }
  //  qDebug("misses++");
  misses++;

  Pattern* pat = nullptr;
  MatchObject* ret = nullptr;
  if (match.re) {
    pat = this;
    ret = match.find(data, pos);
  } else if (begin.re) {
    pat = this;
    ret = begin.find(data, pos);
  } else if (!include.isEmpty()) {
    QChar z = include[0];
    // # means an item name in the repository
    if (z == '#') {
      QString key = include.mid(1, include.length() - 1);
      if (owner->repository.contains(key)) {
        //        qDebug("include %s", qPrintable(include));
        Pattern* p2 = owner->repository.value(key);
        auto pair = p2->cache(data, pos);
        pat = pair.first;
        ret = pair.second;
      } else {
        qWarning() << "Not found in repository:" << include;
      }
    } else if (z == '$') {
      // todo: implement tmLanguage $ include directives
      qWarning() << "Unhandled include directive:" << include;
      // external syntax definitions e.g. source.c++
    } else if (Language* l = LanguageProvider::languageFromScope(include)) {
      return l->rootPattern->cache(data, pos);
    } else {
      if (!failed.contains(include)) {
        qWarning() << "Include directive " + include + " failed";
      }
      failed[include] = true;
    }
  } else {
    auto pair = firstMatch(data, pos);
    pat = pair.first;
    ret = pair.second;
  }
  cachedData = data;
  cachedMatch = ret;
  cachedPat = pat;

  return std::make_pair(pat, ret);
}

Node* Pattern::createNode(const QString& data, int pos, DataSource* d, MatchObject* mo) {
  //  qDebug() << "createNode. mo:" << *mo;
  Node* ret = new Node(name, Region((*mo)[0], (*mo)[1]), d);

  if (match.re) {
    createCaptureNodes(data, pos, d, mo, ret, captures);
  }
  if (!begin.re) {
    ret->updateRange();
    return ret;
  }
  if (beginCaptures.length() > 0) {
    createCaptureNodes(data, pos, d, mo, ret, beginCaptures);
  } else {
    createCaptureNodes(data, pos, d, mo, ret, captures);
  }

  if (!end.re) {
    ret->updateRange();
    return ret;
  }
  bool found = false;
  int i, endPos;

  for (i = ret->range.b, endPos = data.length(); i < data.length();) {
    MatchObject* endmatch = this->end.find(data, i);
    if (endmatch) {
      endPos = (*endmatch)[1];
    } else {
      if (!found) {
        // oops.. no end found at all, set it to the next line
        int e2 = data.indexOf('\n', i);
        if (e2 != -1) {
          endPos = i + e2;
        } else {
          endPos = data.length();
        }
        break;
      } else {
        endPos = i;
        break;
      }
    }

    if (cachedPatterns->length() > 0) {
      // Might be more recursive patterns to apply BEFORE the end is reached
      auto pair = firstMatch(data, i);
      Pattern* pattern2 = pair.first;
      MatchObject* match2 = pair.second;
      if (match2 &&
          ((!endmatch && (*match2)[0] < endPos) ||
           (endmatch && ((*match2)[0] < (*endmatch)[0] ||
                         ((*match2)[0] == (*endmatch)[0] && ret->range.a == ret->range.b))))) {
        found = true;
        Node* r = pattern2->createNode(data, i, d, match2);
        ret->append(r);
        i = r->range.b;
        continue;
      }
    }

    if (endmatch) {
      if (endCaptures.length() > 0) {
        createCaptureNodes(data, i, d, endmatch, ret, endCaptures);
      } else {
        createCaptureNodes(data, i, d, endmatch, ret, captures);
      }
    }
    break;
  }

  ret->range.b = endPos;
  ret->updateRange();
  return ret;
}

void Pattern::createCaptureNodes(const QString&,
                                 int,
                                 DataSource* d,
                                 MatchObject* mo,
                                 Node* parent,
                                 Captures capt) {
  QVector<Region> ranges(mo->length() / 2);
  QVector<int> parentIndex(ranges.length());
  QVector<Node*> parents(parentIndex.length());
  for (int i = 0; i < ranges.length(); i++) {
    ranges[i] = Region((*mo)[i * 2 + 0], (*mo)[i * 2 + 1]);
    if (i < 2) {
      parents[i] = parent;
      continue;
    }
    Region& r = ranges[i];
    for (int j = i - 1; j >= 0; j--) {
      if (ranges[j].covers(r)) {
        parentIndex[i] = j;
        break;
      }
    }
  }

  foreach (const Capture& v, capt) {
    int i = v.key;
    if (i >= parents.length() || ranges[i].a == -1) {
      continue;
    }
    Node* child = new Node(v.name, ranges[i], d);
    parents[i] = child;
    if (i == 0) {
      parent->append(child);
      continue;
    }
    Node* p = nullptr;
    while (!p) {
      i = parentIndex[i];
      p = parents[i];
    }
    p->append(child);
  }
}

void Pattern::tweak(Language* l) {
  owner = l;
  name = name.trimmed();
  if (patterns) {
    foreach (Pattern* p, *patterns) { p->tweak(l); }
  }
}

QString Pattern::toString() const {
  QString formatStr = QString("---------------------------------------\n") % "Name:    %1\n" %
                      "Match:   %2\n" % "Begin:   %3\n" % "End:     %4\n" % "Include: %5\n";
  QString ret =
      formatStr.arg(name).arg(match.toString()).arg(begin.toString()).arg(end.toString()).arg(
          include);
  ret = ret % QString("<Sub-Patterns>\n");
  if (patterns) {
    foreach (Pattern* p, *patterns) {
      ret = ret % QString("\t%1\n").arg(p->toString().replace("\t", "\t\t").replace("\n", "\n\t"));
    }
  }
  ret = ret % QString("</Sub-Patterns>\n---------------------------------------");
  return ret;
}

QVector<QPair<QString, QString>> LanguageProvider::m_langNameAndScopePairs(0);
QMap<QString, QString> LanguageProvider::scopeLangFilePathMap;
QMap<QString, QString> LanguageProvider::extensionLangFilePathMap;

Language* LanguageProvider::languageFromScope(const QString& scope) {
  if (scopeLangFilePathMap.contains(scope)) {
    return languageFromFile(scopeLangFilePathMap.value(scope));
  } else {
    return nullptr;
  }
}

Language* LanguageProvider::languageFromExtension(const QString& ext) {
  if (extensionLangFilePathMap.contains(ext)) {
    return languageFromFile(extensionLangFilePathMap.value(ext));
  } else {
    return nullptr;
  }
}

Language* LanguageProvider::languageFromFile(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("unable to open a file");
    return nullptr;
  }

  QVariant root = PListParser::parsePList(&file);
  if (!root.canConvert<QVariantMap>()) {
    qWarning("root is not dict");
    return nullptr;
  }

  Language* lang = new Language();
  QVariantMap rootMap = root.toMap();

  // fileTypes
  const QString fileTypes = "fileTypes";
  if (rootMap.contains(fileTypes)) {
    QVariant fileTypesVar = rootMap.value(fileTypes);
    if (fileTypesVar.canConvert<QVariantList>()) {
      QSequentialIterable iterable = fileTypesVar.value<QSequentialIterable>();
      foreach (const QVariant& v, iterable) {
        if (v.canConvert<QString>()) {
          QString ext = v.toString();
          lang->fileTypes.append(ext);
        }
      }
    }
  }

  // firstLineMatch
  const QString firstLineMatch = "firstLineMatch";
  if (rootMap.contains(firstLineMatch)) {
    lang->firstLineMatch = rootMap.value(firstLineMatch).toString();
  }

  // scopeName
  const QString scopeName = "scopeName";
  if (rootMap.contains(scopeName)) {
    lang->scopeName = rootMap.value(scopeName).toString();
  }

  // patterns
  RootPattern* rootPattern = toRootPattern(rootMap);
  lang->rootPattern = rootPattern;

  // repository
  const QString repository = "repository";
  if (rootMap.contains(repository)) {
    QVariantMap repositoryMap = rootMap.value(repository).toMap();
    if (!repository.isEmpty()) {
      QMapIterator<QString, QVariant> i(repositoryMap);
      while (i.hasNext()) {
        i.next();
        QString key = i.key();
        if (i.value().canConvert<QVariantMap>()) {
          QVariantMap subMap = i.value().toMap();
          if (Pattern* pattern = toPattern(subMap)) {
            lang->repository.insert(key, pattern);
          }
        }
      }
    }
  }

  if (!scopeLangFilePathMap.contains(lang->scopeName)) {
    foreach (const QString& ext, lang->fileTypes) { extensionLangFilePathMap[ext] = path; }
    scopeLangFilePathMap[lang->scopeName] = path;
    m_langNameAndScopePairs.append(QPair<QString, QString>(lang->name(), lang->scopeName));
  }

  lang->tweak();
  return lang;
}

void LanguageProvider::loadLanguages() {
  QDir dir("packages");
  Q_ASSERT(dir.exists());
  foreach (const QString& fileName, dir.entryList(QStringList("*.tmLanguage"))) {
    qDebug("loading %s", qPrintable(dir.filePath(fileName)));
    languageFromFile(dir.filePath(fileName));
  }
}

bool Region::covers(const Region& r2) {
  return contains(r2.begin()) && r2.end() <= end();
}

bool Region::contains(int point) {
  return begin() <= point && point <= end();
}

int Region::begin() const {
  return qMin(a, b);
}

int Region::end() const {
  return qMax(a, b);
}

int Region::length() const {
  return end() - begin();
}

MatchObject* Regex::find(const QString& data, int pos) {
  //  qDebug("find. pattern: %s, pos: %d", qPrintable(re->pattern()), pos);
  if (lastIndex > pos) {
    lastFound = 0;
  }
  lastIndex = pos;
  while (lastFound < data.length()) {
    std::unique_ptr<QVector<int>> ret(re->findStringSubmatchIndex(data.right(data.length() - lastFound)));
    if (!ret) {
      break;
    } else if (((*ret)[0] + lastFound) < pos) {
      if ((*ret)[0] == 0) {
        lastFound++;
      } else {
        lastFound += (*ret)[0];
      }
      continue;
    }
    MatchObject* mo = new MatchObject(*ret);
    fix(mo, lastFound);

    return mo;
  }
  return nullptr;
}

QString Regex::toString() const {
  if (re == nullptr) {
    return "null";
  }
  return QString("%1   // %2, %3").arg(re->pattern()).arg(lastIndex).arg(lastFound);
}

void Language::tweak() {
  rootPattern->tweak(this);
  foreach (Pattern* p, repository) { p->tweak(this); }
}

QString Language::toString() const {
  return QString("%1\n%2\n").arg(scopeName).arg(rootPattern->toString());
}

QString Language::name() {
  return rootPattern ? rootPattern->name : "";
}

QString RootPattern::toString() const {
  QString ret;
  if (patterns) {
    foreach (Pattern* pat, *patterns) { ret += QString("\t%1\n").arg(pat->toString()); }
  }
  return ret;
}
