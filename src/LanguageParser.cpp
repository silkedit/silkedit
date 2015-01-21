#include <algorithm>
#include <QStringList>
#include <QStringBuilder>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QDir>

#include "LanguageParser.h"
#include "PListParser.h"

namespace {

const int MAX_ITER_COUNT = 10000;
const QString DEFAULT_SCOPE = "text.plain";

// Clamps v to be in the region of _min and _max
int clamp(int min, int max, int v) { return qMax(min, qMin(max, v)); }

Captures toCaptures(QVariantMap map) {
  Captures captures(0);
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
    foreach(const QVariant & v, iterable) {
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
    pat->patterns.reset(toPatterns(patternsVar));
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
    return new LanguageParser(lang, data);
  } else {
    return nullptr;
  }
}

RootNode* LanguageParser::parse() {
  RootNode* rootNode = new RootNode(this, m_lang->scopeName);
  QVector<Node*> children = parse(Region(0, m_text.length()));
  foreach(Node * child, children) { rootNode->append(child); }

  rootNode->updateRegion();
  return rootNode;
}

// parse in [begin, end) (doensn't include end)
QVector<Node*> LanguageParser::parse(const Region& region) {
  qDebug(
      "parse. region: %s. lang: %s", qPrintable(region.toString()), qPrintable(m_lang->scopeName));
  QTime t;
  t.start();

  int iter = MAX_ITER_COUNT;
  QVector<Node*> nodes(0);
  for (int pos = region.begin(); pos < region.end() && iter > 0; iter--) {
    auto pair = m_lang->rootPattern->find(m_text, pos);
    Pattern* pattern = pair.first;
    QVector<Region>* regions = pair.second;
    //    if (pat && ret) {
    //      qDebug() << "pat:" << *pat << "ret:" << *ret;
    //    }
    int nl = m_text.mid(pos).indexOf(QRegularExpression(R"(\n|\r)"));
    if (nl != -1) {
      nl += pos;
    }

    if (!regions) {
      break;
    } else if (nl > 0 && nl <= (*regions)[0].begin()) {
      pos = nl;
      while (pos < m_text.length() && (m_text[pos] == '\n' || m_text[pos] == '\r')) {
        pos++;
      }
    } else {
      Q_ASSERT(regions);
      Node* n = pattern->createNode(m_text, this, *regions);
      if (region.intersects(n->region)) {
        nodes.append(n);
      }
      pos = n->region.end();
    }
  }

  if (iter == 0) {
    throw "reached maximum number of iterations";
  }

  qDebug("parse finished. elapsed: %d ms", t.elapsed());
  return nodes;
}

QString LanguageParser::getData(int a, int b) {
  a = clamp(0, m_text.length(), a);
  b = clamp(0, m_text.length(), b);
  return m_text.mid(a, b - a);
}

void LanguageParser::clearCache() {
  if (m_lang) {
    m_lang->clearCache();
  }
}

LanguageParser::LanguageParser(Language* lang, const QString& str) : m_lang(lang), m_text(str) {}

Node::Node(LanguageParser* p_p, const QString& p_name) {
  parser = p_p;
  name = p_name;
}

Node::Node(const QString& p_name, Region p_region, LanguageParser* p_p) {
  name = p_name;
  region = p_region;
  parser = p_p;
}

void Node::append(Node* child) {
  //  children.append(child);
  children.push_back(std::move(std::unique_ptr<Node>(child)));
}

Region Node::updateRegion() {
  for (auto& child : children) {
    //  foreach (Node* child, children) {
    Region curr = child.get()->updateRegion();
    if (curr.begin() < region.begin()) {
      region.setBegin(curr.begin());
    }
    if (curr.end() > region.end()) {
      region.setEnd(curr.end());
    }
  }
  return region;
}

QString Node::toString() const { return format(""); }

void Node::adjust(int pos, int delta) {
  region.adjust(pos, delta);
  for (auto& child : children) {
    child->adjust(pos, delta);
  }
}

QString Node::format(QString indent) const {
  if (isLeaf()) {
    return indent + QString("%1-%2: \"%3\" - Data: \"%4\"\n")
                        .arg(region.begin())
                        .arg(region.end())
                        .arg(name)
                        .arg(data());
  }
  QString ret;
  ret = ret % indent % QString("%1-%2: \"%3\"\n").arg(region.begin()).arg(region.end()).arg(name);
  indent += "\t";
  for (auto& child : children) {
    ret = ret % child.get()->format(indent);
  }
  return ret;
}

QString Node::data() const { return parser->getData(region.begin(), region.end()); }

Pattern::Pattern() : Pattern("") {}

Pattern::Pattern(const QString& p_include)
    : include(p_include),
      lang(nullptr),
      cachedPattern(nullptr),
      cachedPatterns(nullptr),
      cachedRegions(nullptr) {}

std::pair<Pattern*, QVector<Region>*> Pattern::searchInPatterns(const QString& str, int beginPos) {
  //  qDebug("firstMatch. pos: %d", pos);
  int startIdx = -1;
  Pattern* resultPattern = nullptr;
  QVector<Region>* resultRegions = nullptr;
  int i = 0;
  while (i < cachedPatterns->length()) {
    auto pair = (*cachedPatterns)[i]->find(str, beginPos);
    Pattern* pattern = pair.first;
    QVector<Region>* regions = pair.second;
    if (regions) {
      if (startIdx < 0 || startIdx > (*regions)[0].begin()) {
        startIdx = (*regions)[0].begin();
        resultPattern = pattern;
        resultRegions = regions;
        // This match is right at the start, we're not going to find a better pattern than this, so
        // stop the search
        if ((*regions)[0].begin() == beginPos) {
          break;
        }
      }
      i++;
    } else {
      // If it wasn't found now, it'll never be found, so the pattern can be popped from the cache
      cachedPatterns->removeAt(i);
    }
  }

  return std::make_pair(resultPattern, resultRegions);
}

std::pair<Pattern*, QVector<Region>*> Pattern::find(const QString& str, int beginPos) {
  //  qDebug("cache. pos: %d. data.size: %d", pos, data.size());
  if (!cachedStr.isEmpty() && cachedStr == str) {
    if (!cachedRegions) {
      //      qDebug("cachedMatch is null");
      return std::make_pair(nullptr, nullptr);
    }

    if ((*cachedRegions)[0].begin() >= beginPos && cachedPattern->cachedRegions) {
      //      qDebug("hits++");
      //      hits++;
      return std::make_pair(cachedPattern, cachedRegions);
    }
  } else {
    //    qDebug("cachedPatterns = nullptr");
    cachedPatterns.reset(nullptr);
  }

  if (!cachedPatterns) {
    cachedPatterns.reset(new QVector<Pattern*>(patterns ? patterns->size() : 0));
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
  //  misses++;

  Pattern* pattern = nullptr;
  QVector<Region>* regions = nullptr;
  if (match.regex) {
    pattern = this;
    regions = match.find(str, beginPos);
  } else if (begin.regex) {
    pattern = this;
    regions = begin.find(str, beginPos);
  } else if (!include.isEmpty()) {
    // # means an item name in the repository
    if (include.startsWith('#')) {
      QString key = include.mid(1);
      if (lang->repository.find(key) != lang->repository.end()) {
        //        qDebug("include %s", qPrintable(include));
        Pattern* p2 = lang->repository.at(key).get();
        auto pair = p2->find(str, beginPos);
        pattern = pair.first;
        regions = pair.second;
      } else {
        qWarning() << "Not found in repository:" << include;
      }
      // $self and $base means the current entire syntax definition
    } else if (include == "$self" || include == "$base") {
      //      qDebug("include %s", qPrintable(include));
      return lang->rootPattern->find(str, beginPos);
      // external syntax definitions e.g. source.c++
    } else if (cachedLanguage) {
      return cachedLanguage->rootPattern->find(str, beginPos);
    } else if (Language* anotherLang = LanguageProvider::languageFromScope(include)) {
      cachedLanguage.reset(anotherLang);
      return cachedLanguage->rootPattern->find(str, beginPos);
    } else {
      qWarning() << "Include directive " + include + " failed";
    }
  } else {
    auto pair = searchInPatterns(str, beginPos);
    pattern = pair.first;
    regions = pair.second;
  }

  cachedStr = QStringRef(&str);
  cachedRegions = regions;
  cachedPattern = pattern;

  return std::make_pair(pattern, regions);
}

Node* Pattern::createNode(const QString& str,
                          LanguageParser* parser,
                          const QVector<Region>& regions) {
  Q_ASSERT(!regions.isEmpty());

  //  qDebug() << "createNode. mo:" << *mo;
  Node* node = new Node(name, regions[0], parser);

  if (match.regex) {
    createCaptureNodes(parser, regions, node, captures);
  }

  if (!begin.regex) {
    node->updateRegion();
    return node;
  }

  if (beginCaptures.length() > 0) {
    createCaptureNodes(parser, regions, node, beginCaptures);
  } else {
    createCaptureNodes(parser, regions, node, captures);
  }

  if (!end.regex) {
    node->updateRegion();
    return node;
  }

  bool found = false;
  int i, endPos;

  for (i = node->region.end(), endPos = str.length(); i < str.length();) {
    QVector<Region>* endMatchedRegions = end.find(str, i);
    if (endMatchedRegions) {
      endPos = (*endMatchedRegions)[0].end();
    } else {
      if (!found) {
        // oops.. no end found at all, set it to the next line
        int e2 = str.mid(i).indexOf('\n');
        if (e2 != -1) {
          endPos = i + e2;
        } else {
          endPos = str.length();
        }
        break;
      } else {
        endPos = i;
        break;
      }
    }

    Q_ASSERT(endMatchedRegions);

    if (cachedPatterns->length() > 0) {
      // Might be more recursive patterns to apply BEFORE the end is reached
      auto pair = searchInPatterns(str, i);
      Pattern* patternBeforeEnd = pair.first;
      QVector<Region>* regionsBeforeEnd = pair.second;
      if (regionsBeforeEnd && endMatchedRegions &&
          ((*regionsBeforeEnd)[0].begin() < (*endMatchedRegions)[0].begin() ||
           ((*regionsBeforeEnd)[0].begin() == (*endMatchedRegions)[0].begin() &&
            node->region.isEmpty()))) {
        found = true;
        Node* r = patternBeforeEnd->createNode(str, parser, *regionsBeforeEnd);
        node->append(r);
        i = r->region.end();
        continue;
      }
    }

    if (endCaptures.length() > 0) {
      createCaptureNodes(parser, *endMatchedRegions, node, endCaptures);
    } else {
      createCaptureNodes(parser, *endMatchedRegions, node, captures);
    }

    break;
  }

  node->region.setEnd(endPos);
  node->updateRegion();
  return node;
}

void Pattern::createCaptureNodes(LanguageParser* parser,
                                 QVector<Region> regions,
                                 Node* parent,
                                 Captures captures) {
  QVector<int> parentIndices(regions.length());
  QVector<Node*> parents(parentIndices.length());
  for (int i = 0; i < regions.length(); i++) {
    if (i < 2) {
      parents[i] = parent;
      continue;
    }
    for (int j = i - 1; j >= 0; j--) {
      if (regions[j].fullyCovers(regions[i])) {
        parentIndices[i] = j;
        break;
      }
    }
  }

  foreach(const Capture & v, captures) {
    int i = v.key;
    if (i >= parents.length() || regions[i].begin() == -1) {
      continue;
    }
    Node* child = new Node(v.name, regions[i], parser);
    parents[i] = child;

    if (i == 0) {
      parent->append(child);
      continue;
    }

    Node* p = nullptr;
    while (!p) {
      i = parentIndices[i];
      p = parents[i];
    }
    p->append(child);
  }
}

void Pattern::tweak(Language* l) {
  lang = l;
  name = name.trimmed();
  if (patterns) {
    foreach(Pattern * p, *patterns) { p->tweak(l); }
  }
}

void Pattern::clearCache() {
  cachedPattern = nullptr;
  cachedPatterns.reset(nullptr);
  cachedLanguage.reset(nullptr);
  if (cachedRegions) {
    cachedRegions->clear();
  }
  cachedStr.clear();
  if (patterns) {
    foreach(Pattern * pat, *patterns) { pat->clearCache(); }
  }

  match.lastFound = 0;
  match.lastIndex = 0;
  begin.lastFound = 0;
  begin.lastIndex = 0;
  end.lastFound = 0;
  end.lastIndex = 0;
}

QString Pattern::toString() const {
  QString formatStr = QString("---------------------------------------\n") % "Name:    %1\n" %
                      "Match:   %2\n" % "Begin:   %3\n" % "End:     %4\n" % "Include: %5\n";
  QString ret =
      formatStr.arg(name).arg(match.toString()).arg(begin.toString()).arg(end.toString()).arg(
          include);
  ret = ret % QString("<Sub-Patterns>\n");
  if (patterns) {
    foreach(Pattern * p, *patterns) {
      ret = ret % QString("\t%1\n").arg(p->toString().replace("\t", "\t\t").replace("\n", "\n\t"));
    }
  }
  ret = ret % QString("</Sub-Patterns>\n---------------------------------------");
  return ret;
}

QVector<QPair<QString, QString>> LanguageProvider::m_scopeAndLangNamePairs(0);
QMap<QString, QString> LanguageProvider::m_scopeLangFilePathMap;
QMap<QString, QString> LanguageProvider::m_extensionLangFilePathMap;

Language* LanguageProvider::defaultLanguage() { return languageFromScope(DEFAULT_SCOPE); }

Language* LanguageProvider::languageFromScope(const QString& scope) {
  if (m_scopeLangFilePathMap.contains(scope)) {
    return loadLanguage(m_scopeLangFilePathMap.value(scope));
  } else {
    return nullptr;
  }
}

Language* LanguageProvider::languageFromExtension(const QString& ext) {
  if (m_extensionLangFilePathMap.contains(ext)) {
    return loadLanguage(m_extensionLangFilePathMap.value(ext));
  } else {
    return defaultLanguage();
  }
}

Language* LanguageProvider::loadLanguage(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("unable to open a file %s", qPrintable(path));
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
      foreach(const QVariant & v, iterable) {
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
  lang->rootPattern.reset(toRootPattern(rootMap));

  // repository
  const QString repository = "repository";
  if (rootMap.contains(repository)) {
    QVariantMap repositoryMap = rootMap.value(repository).toMap();
    if (!repository.isEmpty()) {
      QMapIterator<QString, QVariant> iter(repositoryMap);
      while (iter.hasNext()) {
        iter.next();
        QString key = iter.key();
        if (iter.value().canConvert<QVariantMap>()) {
          QVariantMap subMap = iter.value().toMap();
          if (Pattern* pattern = toPattern(subMap)) {
            lang->repository[key] = std::move(std::unique_ptr<Pattern>(pattern));
          }
        }
      }
    }
  }

  if (!m_scopeLangFilePathMap.contains(lang->scopeName)) {
    foreach(const QString & ext, lang->fileTypes) { m_extensionLangFilePathMap[ext] = path; }
    m_scopeLangFilePathMap[lang->scopeName] = path;
    m_scopeAndLangNamePairs.append(QPair<QString, QString>(lang->scopeName, lang->name()));
  }

  lang->tweak();
  return lang;
}

QVector<Region>* Regex::find(const QString& str, int beginPos) {
  //  qDebug("find. pattern: %s, pos: %d", qPrintable(re->pattern()), pos);
  if (lastIndex > beginPos) {
    lastFound = 0;
  }

  lastIndex = beginPos;
  while (lastFound < str.length()) {
    std::unique_ptr<QVector<int>> indices(regex->findStringSubmatchIndex(str.midRef(lastFound)));
    if (!indices) {
      break;
    } else if (((*indices)[0] + lastFound) < beginPos) {
      if ((*indices)[0] == 0) {
        lastFound++;
      } else {
        lastFound += (*indices)[0];
      }
      continue;
    }

    Q_ASSERT(indices);
    Q_ASSERT(indices->length() % 2 == 0);
    for (int i = 0; i < indices->length(); i++) {
      if ((*indices)[i] != -1) {
        (*indices)[i] += lastFound;
      }
    }

    QVector<Region>* regions = new QVector<Region>(indices->length() / 2);
    for (int i = 0; i < indices->length() / 2; i++) {
      (*regions)[i] = Region((*indices)[i * 2], (*indices)[i * 2 + 1]);
    }
    return regions;
  }

  return nullptr;
}

QString Regex::toString() const {
  if (regex == nullptr) {
    return "null";
  }
  return QString("%1   // %2, %3").arg(regex->pattern()).arg(lastIndex).arg(lastFound);
}

void Language::tweak() {
  rootPattern->tweak(this);
  //  foreach (Pattern* p, repository) { p->tweak(this); }
  for (auto& pair : repository) {
    pair.second->tweak(this);
  }
}

QString Language::toString() const {
  return QString("%1\n%2\n").arg(scopeName).arg(rootPattern->toString());
}

QString Language::name() { return rootPattern ? rootPattern->name : ""; }

void Language::clearCache() {
  if (rootPattern) {
    rootPattern->clearCache();
  }
  for (auto& pair : repository) {
    pair.second->clearCache();
  }
}

QString RootPattern::toString() const {
  QString ret;
  if (patterns) {
    foreach(Pattern * pat, *patterns) { ret += QString("\t%1\n").arg(pat->toString()); }
  }
  return ret;
}

RootNode::RootNode(LanguageParser* parser, const QString& name) : Node(parser, name) {}

void RootNode::adjust(int pos, int delta) {
  region.setEnd(region.end() + delta);
  for (auto& child : children) {
    child->adjust(pos, delta);
  }
}

void RootNode::updateChildren(const Region& region, LanguageParser* parser) {
  qDebug("updateChildren. region: %s", qPrintable(region.toString()));
  parser->clearCache();

  Region affectedRegion(region);
  Q_ASSERT(affectedRegion.begin() == region.begin());
  Q_ASSERT(affectedRegion.end() == region.end());

  for (auto it = children.begin(); it != children.end();) {
    qDebug("child region: %s", qPrintable((*it)->region.toString()));
    if ((*it)->region.intersects(region)) {
      //      qDebug() << "affected child:" << (*it)->region;
      // update affected region
      affectedRegion.setBegin(qMin(affectedRegion.begin(), (*it)->region.begin()));
      affectedRegion.setEnd(qMax(affectedRegion.end(), (*it)->region.end()));
      it = children.erase(it);
    } else {
      it++;
    }
  }

  QVector<Node*> newNodes = parser->parse(affectedRegion);
  foreach(Node * node, newNodes) {
    //    qDebug("new node: %s", qPrintable(node->toString()));
    children.push_back(std::move(std::unique_ptr<Node>(node)));
  }
  std::sort(children.begin(),
            children.end(),
            [](const std::unique_ptr<Node>& x,
               const std::unique_ptr<Node>& y) { return x->region.begin() < y->region.begin(); });

  qDebug("new children.size: %d", (int)children.size());
  //  qDebug() << *this;
}
