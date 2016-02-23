#include <memory>
#include <algorithm>
#include <iterator>
#include <vector>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QDir>

#include "LanguageParser.h"
#include "PListParser.h"
#include "Regexp.h"

namespace core {

namespace {

const int MAX_ITER_COUNT = 10000;
const QString DEFAULT_SCOPE = QStringLiteral("text.plain");
const QString HIDE_FROM_USER_KEY = QStringLiteral("hideFromUser");
const QString FILE_TYPES_KEY = QStringLiteral("fileTypes");
const QString FIRST_LINE_MATCH_KEY = QStringLiteral("firstLineMatch");
const QString SCOPE_NAME_KEY = QStringLiteral("scopeName");

// Clamps v to be in the region of _min and _max
int clamp(int min, int max, int v) {
  return qMax(min, qMin(max, v));
}

Captures toCaptures(QVariantMap map) {
  Captures captures(0);
  bool ok;
  QMapIterator<QString, QVariant> i(map);
  static const QString nameStr = QStringLiteral("name");

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

  std::sort(captures.begin(), captures.end(),
            [](const Capture& x, const Capture& y) { return x.key < y.key; });
  return captures;
}

Pattern* toChildPattern(QVariantMap map, Pattern* parent);

QVector<Pattern*>* toPatterns(QVariant patternsVar, Pattern* parent) {
  if (patternsVar.canConvert<QVariantList>()) {
    QVector<Pattern*>* patterns = new QVector<Pattern*>(0);
    QSequentialIterable iterable = patternsVar.value<QSequentialIterable>();
    foreach (const QVariant& v, iterable) {
      if (v.canConvert<QVariantMap>()) {
        patterns->append(toChildPattern(v.toMap(), parent));
      }
    }
    return patterns;
  }
  return nullptr;
}

void toPattern(QVariantMap map, Pattern* pat) {
  // include
  static const QString include = QStringLiteral("include");
  if (map.contains(include)) {
    pat->include = map.value(include).toString();
  }

  // match
  static const QString match = QStringLiteral("match");
  if (map.contains(match)) {
    pat->match.reset(new FixedRegex(map.value(match).toString()));
  }

  // name
  static const QString name = QStringLiteral("name");
  if (map.contains(name)) {
    pat->name = map.value(name).toString();
  }

  // begin
  static const QString begin = QStringLiteral("begin");
  if (map.contains(begin)) {
    pat->begin.reset(new FixedRegex(map.value(begin).toString()));
  }

  // beginCaptures
  static const QString beginCaptures = QStringLiteral("beginCaptures");
  if (map.contains(beginCaptures)) {
    QVariant beginCapturesVar = map.value(beginCaptures);
    if (beginCapturesVar.canConvert<QVariantMap>()) {
      pat->beginCaptures = toCaptures(beginCapturesVar.toMap());
    }
  }

  // contentName
  static const QString contentName = QStringLiteral("contentName");
  if (map.contains(contentName)) {
    pat->contentName = map.value(contentName).toString();
  }

  // end
  static const QString end = QStringLiteral("end");
  if (map.contains(end)) {
    pat->end.reset(Regex::create(map.value(end).toString()));
  }

  // endCaptures
  static const QString endCaptures = QStringLiteral("endCaptures");
  if (map.contains(endCaptures)) {
    QVariant endCapturesVar = map.value(endCaptures);
    if (endCapturesVar.canConvert<QVariantMap>()) {
      pat->endCaptures = toCaptures(endCapturesVar.toMap());
    }
  }

  // captures
  static const QString captures = QStringLiteral("captures");
  if (map.contains(captures)) {
    QVariant capturesVar = map.value(captures);
    if (capturesVar.canConvert<QVariantMap>()) {
      pat->captures = toCaptures(capturesVar.toMap());
    }
  }

  // patterns
  static const QString patternsStr = QStringLiteral("patterns");
  if (map.contains(patternsStr)) {
    QVariant patternsVar = map.value(patternsStr);
    pat->patterns.reset(toPatterns(patternsVar, pat));
  }

  // repository
  static const QString repositoryStr = QStringLiteral("repository");
  if (map.contains(repositoryStr)) {
    QVariantMap repositoryMap = map.value(repositoryStr).toMap();
    QMapIterator<QString, QVariant> iter(repositoryMap);
    while (iter.hasNext()) {
      iter.next();
      QString key = iter.key();
      if (iter.value().canConvert<QVariantMap>()) {
        QVariantMap subMap = iter.value().toMap();
        if (Pattern* pattern = toChildPattern(subMap, pat)) {
          pat->repository[key] = std::move(std::unique_ptr<Pattern>(pattern));
        }
      }
    }
  }
}

RootPattern* toRootPattern(QVariantMap map) {
  RootPattern* pat = new RootPattern();
  toPattern(map, pat);
  return pat;
}

// todo: check ownership
Pattern* toChildPattern(QVariantMap map, Pattern* parent) {
  Pattern* pat = new Pattern(parent);
  toPattern(map, pat);
  return pat;
}

template <typename _OutputIter>
_OutputIter escapeRegexp(QChar const* it, QChar const* last, _OutputIter out) {
  static QString special = QStringLiteral("\\|([{}]).?*+^$");
  while (it != last) {
    if (special.contains(*it)) {
      *out++ = '\\';
    }
    *out++ = *it++;
  }
  return out;
}

QString expandBackReferences(QString const& ptrn, QList<QStringRef> m) {
  bool escape = false;
  QString res;
  for (auto const& it : ptrn) {
    if (escape && it.isDigit()) {
      int i = it.digitValue();
      if (i < m.size())
        escapeRegexp(m[i].begin(), m[i].end(), std::back_inserter(res));
      escape = false;
      continue;
    }

    if (escape)
      res += '\\';
    if (!(escape = !escape && it == '\\'))
      res += it;
  }
  return res;
}

QList<QStringRef> getCaptures(const QStringRef& str, QVector<Region> regions) {
  QList<QStringRef> capturedStrs;
  for (const auto& reg : regions) {
    capturedStrs.append(str.mid(reg.begin(), reg.length()));
  }
  return capturedStrs;
}

Pattern* findInRepository(Pattern* pattern, const QString& key) {
  if (!pattern) {
    return nullptr;
  }

  if (pattern->repository.find(key) != pattern->repository.end()) {
    return pattern->repository.at(key).get();
  }

  return findInRepository(pattern->parent, key);
}

bool inSameLine(const QString& text, int begin, int end) {
  return !text.midRef(begin, end - begin).contains('\n');
}
}

LanguageParser* LanguageParser::create(const QString& scopeName, const QString& data) {
  if (Language* lang = LanguageProvider::languageFromScope(scopeName)) {
    return new LanguageParser(lang, data);
  } else {
    return nullptr;
  }
}

std::unique_ptr<RootNode> LanguageParser::parse() {
  std::unique_ptr<RootNode> rootNode(new RootNode(this, m_lang->scopeName));
  std::vector<std::unique_ptr<Node>> children = parse(Region(0, m_text.length()));

  std::for_each(
      std::make_move_iterator(std::begin(children)), std::make_move_iterator(std::end(children)),
      [&](decltype(children)::value_type&& child) { rootNode->append(std::move(child)); });

  // root node covers everything
  rootNode->region = Region(0, m_text.length());
  return std::move(rootNode);
}

// parse in [begin, end) (doensn't include end)
std::vector<std::unique_ptr<Node>> LanguageParser::parse(const Region& region) {
  qDebug("parse. region: %s. lang: %s", qPrintable(region.toString()),
         qPrintable(m_lang->scopeName));
  QTime t;
  t.start();

  int iter = MAX_ITER_COUNT;
  std::vector<std::unique_ptr<Node>> nodes(0);
  int prevPos;

  for (int pos = region.begin(); pos < region.end() && iter > 0; iter--) {
    prevPos = pos;
    // Try to find a root pattern in m_text from pos.
    auto pair = m_lang->rootPattern->find(m_text, pos);
    Pattern* pattern = pair.first;

    // This regions could include empty region
    // e.g. /(^[ \t]+)?(?=#)/ in SQL.plist
    boost::optional<QVector<Region>> regions = pair.second;

    int newlinePos = m_text.indexOf(QRegularExpression(R"(\n|\r)"), pos);

    if (!regions) {
      break;
    } else if (newlinePos > 0 && newlinePos <= (*regions)[0].begin()) {
      pos = newlinePos;
      while (pos < m_text.length() && (m_text[pos] == '\n' || m_text[pos] == '\r')) {
        pos++;
      }
    } else {
      Q_ASSERT(regions);
      std::unique_ptr<Node> n = pattern->createNode(m_text, this, *regions);
      pos = n->region.end();
      if (region.intersects(n->region)) {
        nodes.push_back(std::move(n));
      }
    }

    // pos doesn't increase. Increment pos to avoid infinite loop
    if (prevPos == pos) {
      pos++;
    }
  }

  if (iter == 0) {
    throw std::runtime_error("reached maximum number of iterations");
  }

  qDebug("parse finished. elapsed: %d ms", t.elapsed());
  return std::move(nodes);
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

int LanguageParser::beginOfLine(int pos) {
  if (pos < 0 || m_text.size() - 1 < pos) {
    return -1;
  }

  Q_ASSERT(0 <= pos && pos <= m_text.size() - 1);

  if (m_text[pos] == '\n') {
    pos--;
  }

  while (pos >= 0 && m_text[pos] != '\n') {
    pos--;
  }

  // begin of the line is 0 or the next char of '\n'
  pos++;

  return pos;
}

// When QTextDocument#setPlainText is called, two contentsChange events are fired. But first one has
// wrong charsRemoved and charsAdded (actual charsRemoved + 1 and actual charsAdded + 1 respectively), so we need to workaround it in this method.
// contentsChange(pos: 0, charsRemoved: 6716, charsAdded: 0)
// contentsChange(pos: 0, charsRemoved: 0, charsAdded: 6715)
// https://bugreports.qt.io/browse/QTBUG-3495
int LanguageParser::endOfLine(int pos) {
  if (pos < 0) {
    return -1;
  }

  while (pos < m_text.size() && m_text[pos] != '\n') {
    pos++;
  }

  if (m_text.size() <= pos) {
    return m_text.size() - 1;
  }

  return pos;
}

LanguageParser::LanguageParser(Language* lang, const QString& str) : m_lang(lang), m_text(str) {}

Node::Node(LanguageParser* p_p, const QString& p_name) : name(p_name), parser(p_p) {
  Q_ASSERT(p_p);
}

Node::Node(const QString& p_name, Region p_region, LanguageParser* p_p)
    : region(p_region), name(p_name), parser(p_p) {
  Q_ASSERT(p_p);
}

void Node::append(std::unique_ptr<Node> child) {
  // skip empty region
  if (child && !child->region.isEmpty()) {
    children.push_back(std::move(child));
  }
}

Region Node::updateRegion() {
  for (auto& child : children) {
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

QString Node::toString() const {
  return format("");
}

void Node::adjust(int pos, int delta) {
  region.adjust(pos, delta);
  for (auto& child : children) {
    child->adjust(pos, delta);
  }
}

QString Node::format(QString indent) const {
  if (isLeaf()) {
    return indent +
           QStringLiteral("%1-%2: \"%3\" - Data: \"%4\"\n")
               .arg(region.begin())
               .arg(region.end())
               .arg(name)
               .arg(data());
  }
  QString ret =
      indent + QStringLiteral("%1-%2: \"%3\"\n").arg(region.begin()).arg(region.end()).arg(name);
  indent += "  ";
  for (auto& child : children) {
    ret = ret + child.get()->format(indent);
  }
  return ret;
}

QString Node::data() const {
  return parser->getData(region.begin(), region.end());
}

bool Regex::hasBackReference(const QString& str) {
  bool escape = false;
  for (const QChar& ch : str) {
    if (escape && ch.isDigit()) {
      return true;
    }
    escape = !escape && ch == '\\';
  }
  return false;
}

boost::optional<QVector<Region>> Regex::find(Regexp* regex,
                                             const QString& str,
                                             int beginPos,
                                             int endPos) {
  //  qDebug("find. pattern: %s, pos: %d", qPrintable(re->pattern()), pos);

  if (!regex) {
    return boost::none;
  }

  QVector<int> indices = regex->findStringSubmatchIndex(str, beginPos, endPos, false);
  if (!indices.isEmpty()) {
    Q_ASSERT(!indices.isEmpty());
    Q_ASSERT(indices.size() % 2 == 0);

    QVector<Region> regions(indices.size() / 2);
    for (int i = 0; i < indices.size() / 2; i++) {
      regions[i] = Region(indices.at(i * 2), indices.at(i * 2 + 1));
    }
    return regions;
  }
  return boost::none;
}

Regex* Regex::create(const QString& pattern) {
  if (hasBackReference(pattern)) {
    return new RegexWithBackReference(pattern);
  } else {
    return new FixedRegex(pattern);
  }
}

Pattern::Pattern(Pattern* parent)
    : lang(nullptr), cachedResultPattern(nullptr), cachedPatterns(nullptr), parent(parent) {}

std::pair<Pattern*, boost::optional<QVector<Region>>> Pattern::searchInPatterns(const QString& str,
                                                                                int beginPos) {
  //  qDebug("firstMatch. pos: %d", pos);
  int startIdx = -1;
  Pattern* resultPattern = nullptr;
  boost::optional<QVector<Region>> resultRegions;
  int i = 0;
  // todo: think about better cache not to use this
  QVector<Pattern*> backslashGPatterns;

  while (i < cachedPatterns->length()) {
    auto pair = (*cachedPatterns)[i]->find(str, beginPos);
    Pattern* pattern = pair.first;
    boost::optional<QVector<Region>> regions = pair.second;
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
      // But don't remove pattern with \G because it may match in the future with another \G
      if ((*cachedPatterns)[i]->match && (*cachedPatterns)[i]->match->pattern().contains(R"(\G)")) {
        backslashGPatterns.append((*cachedPatterns)[i]);
        (*cachedPatterns)[i]->clearCache();
      }

      cachedPatterns->removeAt(i);
    }
  }

  for (auto p : backslashGPatterns) {
    cachedPatterns->prepend(p);
  }
  return std::make_pair(resultPattern, resultRegions);
}

/**
 * @brief Pattern::find
 * Find this pattern in the str from beginPos.
 *
 * @param str
 * @param beginPos
 * @return A pair of pattern and regions found in str. The regions may include an empty region [0,0]
 */
std::pair<Pattern*, boost::optional<QVector<Region>>> Pattern::find(const QString& str,
                                                                    int beginPos) {
  //  qDebug("cache. pos: %d. data.size: %d", pos, data.size());

  if (!cachedStr.isEmpty() && cachedStr == str) {
    if (!cachedResultRegions) {
      //      qDebug("cachedMatch is null");
      return std::make_pair(nullptr, boost::none);
    }

    if ((*cachedResultRegions)[0].begin() >= beginPos && cachedResultPattern->cachedResultRegions) {
      //      qDebug("hits++");
      //      hits++;
      return std::make_pair(cachedResultPattern, cachedResultRegions);
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
  boost::optional<QVector<Region>> regions;
  if (match) {
    pattern = this;
    regions = match->find(str, beginPos);
  } else if (begin) {
    pattern = this;
    regions = begin->find(str, beginPos);
  } else if (!include.isEmpty()) {
    // # means an item name in the repository
    if (include.startsWith('#')) {
      QString key = include.mid(1);
      if (auto p2 = findInRepository(this, key)) {
        //        qDebug("include %s", qPrintable(include));
        auto pair = p2->find(str, beginPos);
        pattern = pair.first;
        regions = pair.second;
      } else {
        qWarning() << "Not found in repository:" << include;
      }
      // $self means the current syntax definition
    } else if (include == "$self") {
      return lang->rootPattern->find(str, beginPos);
      // $base equals $self if it doesn't have a parent. When it does, $base means parent syntax
      // e.g. When source.c++ includes source.c, "include $base" in source.c means including
      // source.c++
    } else if (include == "$base" && lang->baseLanguage) {
      return lang->baseLanguage->rootPattern->find(str, beginPos);
    } else if (includedLanguage) {
      return includedLanguage->rootPattern->find(str, beginPos);
      // external syntax definitions e.g. source.c++
    } else if (Language* includedLang = LanguageProvider::languageFromScope(include)) {
      includedLanguage.reset(includedLang);
      includedLanguage->baseLanguage = lang;
      return includedLanguage->rootPattern->find(str, beginPos);
    } else {
      qWarning() << "Include directive " + include + " failed";
    }
  } else {
    auto pair = searchInPatterns(str, beginPos);
    pattern = pair.first;
    regions = pair.second;
  }

  cachedStr = QStringRef(&str);
  cachedResultRegions = regions;
  cachedResultPattern = pattern;

  return std::make_pair(pattern, regions);
}

std::unique_ptr<Node> Pattern::createNode(const QString& str,
                                          LanguageParser* parser,
                                          const QVector<Region>& regions) {
  Q_ASSERT(!regions.isEmpty());

  //  qDebug() << "createNode. mo:" << *mo;

  std::unique_ptr<Node> node(new Node(name, regions[0], parser));

  if (match) {
    createCaptureNodes(parser, regions, node.get(), captures);
  }

  if (!begin) {
    node->updateRegion();
    return std::move(node);
  }

  if (beginCaptures.length() > 0) {
    createCaptureNodes(parser, regions, node.get(), beginCaptures);
  } else {
    createCaptureNodes(parser, regions, node.get(), captures);
  }

  if (!end) {
    node->updateRegion();
    return std::move(node);
  }

  bool found = false;
  int i, endPos;

  // Store cached result regions because searchInPatterns may overwrite it.
  // If it's overwritten, next iteration in for loop has different result regions compared by
  // previous iteration
  // Don't cache cachedPatterns. It's supposed to be overwritten in searchInPatterns.
  auto tmpCachedRegions = cachedResultRegions;

  for (i = node->region.end(), endPos = str.length(); i < str.length();) {
    // end region can include an empty region [0,0]
    boost::optional<QVector<Region>> endMatchedRegions;
    if (tmpCachedRegions) {
      endMatchedRegions = end->find(str, i, -1, getCaptures(cachedStr, *tmpCachedRegions));
    } else {
      endMatchedRegions = end->find(str, i);
    }
    if (endMatchedRegions) {
      endPos = (*endMatchedRegions)[0].end();
    } else {
      if (!found) {
        // If there is no match for the end pattern, the end of the document is used
        endPos = str.length();
      } else {
        endPos = i;
      }

      // set endMatchedRegions empty region at endPos
      endMatchedRegions = QVector<Region>{Region(endPos, endPos)};
    }

    Q_ASSERT(endMatchedRegions);

    // check if begin and end are in a same line
    bool isEndInSameLine = inSameLine(str, i, (*endMatchedRegions)[0].begin());

    // Search patterns between begin and end
    if (cachedPatterns && cachedPatterns->length() > 0) {
      std::pair<Pattern*, boost::optional<QVector<Region>>> pair;
      /*
       In the following rule, punctuation.separator.continuation.c exceeds the end pos of end
       pattern
       because $ doesn't include \n
       But both TextMate and Sublime allow this, so we need to support this behavior as well.

       * <key>end</key>
         <string>(?=(?://|/\*))|$</string>
         <key>name</key>
         <string>meta.preprocessor.macro.c</string>
         <key>patterns</key>
         <array>
           <dict>
             <key>match</key>
             <string>(?&gt;\\\s*\n)</string>
             <key>name</key>
             <string>punctuation.separator.continuation.c</string>
       */
      pair = searchInPatterns(str, i);

      Pattern* patternBeforeEnd = pair.first;
      boost::optional<QVector<Region>> regionsBeforeEnd = pair.second;
      if (regionsBeforeEnd && endMatchedRegions &&
          // If end pattern exists in a same line, the begin pos of patterns must not exceed the
          // beginning of the end pattern
          (!isEndInSameLine || (*regionsBeforeEnd)[0].begin() < (*endMatchedRegions)[0].begin()) &&
          ((*regionsBeforeEnd)[0].begin() < (*endMatchedRegions)[0].begin() ||
           ((*regionsBeforeEnd)[0].begin() == (*endMatchedRegions)[0].begin() &&
            node->region.isEmpty()))) {
        found = true;
        std::unique_ptr<Node> r(patternBeforeEnd->createNode(str, parser, *regionsBeforeEnd));
        i = r->region.end();

        // If r->region is empty, it leads infinite loop without i++;
        if (r->region.isEmpty()) {
          i++;
        }
        node->append(std::move(r));

        /*
         e.g. text for match

         #define foo(a) \

         rule for match (from C.tmLanguage)

         <key>end</key>
         <string>(?=(?://|/\*))|$</string>
         <key>name</key>
         <string>meta.preprocessor.macro.c</string>
         <key>patterns</key>
         <array>
           <dict>
             <key>match</key>
             <string>(?&gt;\\\s*\n)</string>
             <key>name</key>
             <string>punctuation.separator.continuation.c</string>


         In this case, puctuationseparator.continuation.c matches \n and $ end pattern also matches
         \n
         We need to stop searching end pattern further when i >= endPos and str[i - 1] == '\n'
         Otherwise, end pattern matches every line (because i becomes the beginning of the next
         line)
         */
        if (i < endPos || str[i - 1] != '\n') {
          continue;
        }
      }
    }

    // set contentName
    if (!contentName.isEmpty()) {
      std::unique_ptr<Node> newNode(new Node(
          contentName, Region(node->region.end(), (*endMatchedRegions)[0].begin()), parser));
      node->append(std::move(newNode));
    }

    if (endCaptures.length() > 0) {
      createCaptureNodes(parser, *endMatchedRegions, node.get(), endCaptures);
    } else {
      createCaptureNodes(parser, *endMatchedRegions, node.get(), captures);
    }

    break;
  }

  node->region.setEnd(endPos);
  node->updateRegion();
  return std::move(node);
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

  foreach (const Capture& v, captures) {
    int i = v.key;
    if (i >= parents.length() || regions[i].begin() == -1) {
      continue;
    }
    std::unique_ptr<Node> child(new Node(v.name, regions[i], parser));
    parents[i] = child.get();

    if (i == 0) {
      parent->append(std::move(child));
      continue;
    }

    Node* p = nullptr;
    while (!p) {
      i = parentIndices[i];
      p = parents[i];
    }
    p->append(std::move(child));
  }
}

void Pattern::tweak(Language* l) {
  lang = l;
  name = name.trimmed();
  if (patterns) {
    foreach (Pattern* p, *patterns) { p->tweak(l); }
  }
  for (auto& pair : repository) {
    pair.second->tweak(lang);
  }
}

void Pattern::clearCache() {
  cachedResultPattern = nullptr;
  cachedPatterns.reset(nullptr);
  includedLanguage.reset(nullptr);
  cachedResultRegions = boost::none;
  cachedStr.clear();
  if (patterns) {
    foreach (Pattern* pat, *patterns) { pat->clearCache(); }
  }
  for (auto& pair : repository) {
    pair.second->clearCache();
  }
}

QVector<QPair<QString, QString>> LanguageProvider::m_scopeAndLangNamePairs(0);
QMap<QString, QString> LanguageProvider::m_scopeLangFilePathMap;
QMap<QString, QString> LanguageProvider::m_extensionLangFilePathMap;

Language* LanguageProvider::defaultLanguage() {
  return languageFromScope(DEFAULT_SCOPE);
}

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
    return nullptr;
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
  if (rootMap.contains(FILE_TYPES_KEY)) {
    QVariant fileTypesVar = rootMap.value(FILE_TYPES_KEY);
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

  // hideFromUser
  if (rootMap.contains(HIDE_FROM_USER_KEY)) {
    QVariant var = rootMap.value(HIDE_FROM_USER_KEY);
    if (var.canConvert<bool>()) {
      lang->hideFromUser = var.toBool();
    }
  }

  // firstLineMatch
  if (rootMap.contains(FIRST_LINE_MATCH_KEY)) {
    lang->firstLineMatch = rootMap.value(FIRST_LINE_MATCH_KEY).toString();
  }

  // scopeName
  if (rootMap.contains(SCOPE_NAME_KEY)) {
    lang->scopeName = rootMap.value(SCOPE_NAME_KEY).toString();
  }

  // patterns
  lang->rootPattern.reset(toRootPattern(rootMap));

  if (!m_scopeLangFilePathMap.contains(lang->scopeName)) {
    foreach (const QString& ext, lang->fileTypes) { m_extensionLangFilePathMap[ext] = path; }
    m_scopeLangFilePathMap[lang->scopeName] = path;
    m_scopeAndLangNamePairs.append(QPair<QString, QString>(lang->scopeName, lang->name()));
  }

  lang->tweak();
  return lang;
}

FixedRegex::FixedRegex(const QString& pattern) : Regex(), regex(Regexp::compile(pattern)) {}

QString FixedRegex::pattern() {
  return regex ? regex->pattern() : "";
}

boost::optional<QVector<Region>> FixedRegex::find(const QString& str,
                                                  int beginPos,
                                                  int endPos,
                                                  QList<QStringRef>) {
  return Regex::find(regex.get(), str, beginPos, endPos);
}

void Language::tweak() {
  rootPattern->tweak(this);
}

QString Language::name() {
  return rootPattern ? rootPattern->name : "";
}

void Language::clearCache() {
  if (rootPattern) {
    rootPattern->clearCache();
  }
}

RootNode::RootNode(LanguageParser* parser, const QString& name) : Node(parser, name) {}

void RootNode::adjust(int pos, int delta) {
  region.setEnd(region.end() + delta);
  for (auto& child : children) {
    child->adjust(pos, delta);
  }
}

Region RootNode::updateChildren(const Region& region, LanguageParser* parser) {
  qDebug() << "updateChildren. region:" << region.toString();
  parser->clearCache();

  Region affectedRegion(region);
  Q_ASSERT(affectedRegion.begin() == region.begin());
  Q_ASSERT(affectedRegion.end() == region.end());

  for (auto it = children.begin(); it != children.end();) {
//    qDebug() << "child region:" << (*it)->region.toString();
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

  std::vector<std::unique_ptr<Node>> newNodes = parser->parse(affectedRegion);

  if (newNodes.size() > 0) {
    // Extend affectedRegion based on newNodes
    affectedRegion.setBegin(newNodes[0]->region.begin());
    affectedRegion.setEnd(newNodes[newNodes.size() - 1]->region.end());
  }

  // Remove children that intersect affectedRegion
  for (auto it = children.begin(); it != children.end();) {
    if ((*it)->region.intersects(affectedRegion)) {
      it = children.erase(it);
    } else {
      it++;
    }
  }

  std::for_each(
      std::make_move_iterator(newNodes.begin()), std::make_move_iterator(newNodes.end()),
      [&](decltype(newNodes)::value_type&& node) { children.push_back(std::move(node)); });

  std::sort(children.begin(), children.end(),
            [](const std::unique_ptr<Node>& x, const std::unique_ptr<Node>& y) {
              return x->region.begin() < y->region.begin();
            });

  qDebug("new children.size: %d", (int)children.size());
  //  qDebug().noquote() << *this;

  return affectedRegion;
}

boost::optional<QVector<Region>> RegexWithBackReference::find(const QString& str,
                                                              int beginPos,
                                                              int endPos,
                                                              QList<QStringRef> capturedStrs) {
  auto regex = Regexp::compile(expandBackReferences(patternStr, capturedStrs));
  if (!regex) {
    qWarning() << "failed to compile" << expandBackReferences(patternStr, capturedStrs);
    return boost::none;
  }
  return Regex::find(regex.get(), str, beginPos, endPos);
}

}  // namespace core
