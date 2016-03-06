#include <memory>
#include <tuple>
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

Pattern* toChildPattern(QVariantMap map, Pattern* parent, Language* lang);

QVector<Pattern*>* toPatterns(QVariant patternsVar, Pattern* parent, Language* lang) {
  if (patternsVar.canConvert<QVariantList>()) {
    QVector<Pattern*>* patterns = new QVector<Pattern*>(0);
    QSequentialIterable iterable = patternsVar.value<QSequentialIterable>();
    foreach (const QVariant& v, iterable) {
      if (v.canConvert<QVariantMap>()) {
        patterns->append(toChildPattern(v.toMap(), parent, lang));
      }
    }
    return patterns;
  }
  return nullptr;
}

void toPattern(QVariantMap map, Pattern* pat, Language* lang) {
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
    pat->name = map.value(name).toString().trimmed();
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
    pat->contentName = map.value(contentName).toString().trimmed();
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
    pat->patterns.reset(toPatterns(patternsVar, pat, lang));
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
        if (Pattern* pattern = toChildPattern(subMap, pat, lang)) {
          pat->repository[key] = std::move(std::unique_ptr<Pattern>(pattern));
        }
      }
    }
  }
}

RootPattern* toRootPattern(QVariantMap map, Language* lang) {
  RootPattern* pat = new RootPattern(lang);
  toPattern(map, pat, lang);
  return pat;
}

// todo: check ownership
Pattern* toChildPattern(QVariantMap map, Pattern* parent, Language* lang) {
  Pattern* pat = new Pattern(lang, parent);
  toPattern(map, pat, lang);
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

// Returns the expanded region that covers nodes that intersect region
boost::optional<std::tuple<int, int>> coveringIndices(QList<Node> nodes, Region region) {
  int begin = INT_MAX, end = -1;
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].region.intersects(region)) {
      begin = qMin(begin, i);
      end = qMax(end, i);
    }
  }

  if (end == -1) {
    return boost::none;
  }

  return std::make_tuple(begin, end);
}
}

LanguageParser* LanguageParser::create(const QString& scopeName, const QString& data) {
  if (auto lang = LanguageProvider::languageFromScope(scopeName)) {
    return new LanguageParser(std::move(std::unique_ptr<Language>(lang)), data);
  } else {
    return nullptr;
  }
}

boost::optional<RootNode> LanguageParser::parse() {
  Q_ASSERT(isIdle());
  setState(State::FullParsing);

  const auto& txt = text();
  RootNode rootNode(m_lang->scopeName);
  auto result = parse(txt, QList<Node>(), Region(0, txt.length()));
  auto children = std::get<0>(result);

  if (isCancelRequested()) {
    setState(State::Idle);
    return boost::none;
  }

  std::for_each(std::begin(children), std::end(children),
                [&](const Node& child) { rootNode.append(child); });

  // root node covers everything
  rootNode.region = Region(0, txt.length());
  setState(State::Idle);
  return rootNode;
}

// parse in [begin, end) (doensn't include end)
boost::optional<std::tuple<QList<Node>, Region>> LanguageParser::parse(QList<Node> children,
                                                                       Region region) {
  Q_ASSERT(isIdle());
  setState(State::PartialParsing);
  auto result = parse(text(), children, region);
  auto nodes = std::get<0>(result);
  auto parsedRegion = std::get<1>(result);

  if (isCancelRequested()) {
    setState(State::Idle);
    return boost::none;
  }

  setState(State::Idle);
  return std::make_tuple(nodes, parsedRegion);
}

// The main thread MUST NOT run this method because this method calls
// QCoreApplication::processEvents to cancel
std::tuple<QList<Node>, Region> LanguageParser::parse(const QString& text,
                                                      QList<Node> children,
                                                      Region region) {
  qDebug() << "parse. region:" << region.toString() << "lang:" << m_lang->scopeName;
  int endChildIndex = -1;
  if (const auto& indices = coveringIndices(children, region)) {
    int beginChildIndex = std::get<0>(*indices);
    endChildIndex = std::get<1>(*indices);

    // expand region to cover affected children
    region = Region(qMin(region.begin(), children[beginChildIndex].region.begin()),
                    qMax(region.end(), children[endChildIndex].region.end()));
  }

  QTime t;
  t.start();

  clearCache();

  QList<Node> nodes;
  int prevPos;
  const QLatin1Char lf('\n');
  const QLatin1Char cr('\r');

  for (int pos = region.begin(); pos < region.end();) {
    // check if an another parse request comes before finishing this parse. In that case, cancel
    // this parse
    QCoreApplication::processEvents();
    if (isCancelRequested()) {
      return std::make_tuple(QList<Node>(), region);
    }

    prevPos = pos;
    // Try to find a root pattern in text from pos.
    const auto& pair = m_lang->rootPattern->find(text, pos);
    Pattern* pattern = pair.first;

    // This regions could include empty region
    // e.g. /(^[ \t]+)?(?=#)/ in SQL.plist
    boost::optional<QVector<Region>> regions = pair.second;

    int newlinePos = text.indexOf(lf, pos);
    if (newlinePos > 0 && text[newlinePos - 1] == cr) {
      newlinePos--;
    }

    if (!regions) {
      break;
    } else if (newlinePos > 0 && newlinePos <= (*regions)[0].begin()) {
      pos = newlinePos;
      while (pos < text.length() && (text[pos] == lf || text[pos] == cr)) {
        pos++;
      }
    } else {
      Q_ASSERT(regions);
      Node node = pattern->createNode(text, *regions);
      const auto& newNodeRegion = node.region;
      pos = newNodeRegion.end();

      // Expand region to parse more children
      if (0 <= endChildIndex && pos > children[endChildIndex].region.end() &&
          endChildIndex + 1 < children.size()) {
        endChildIndex++;
        region.setEnd(children[endChildIndex].region.end());
      }

      if (region.intersects(newNodeRegion)) {
        nodes.push_back(node);
      }
    }

    // pos doesn't increase. Increment pos to avoid infinite loop
    if (prevPos == pos) {
      pos++;
    }
  }

  qDebug("parse finished. elapsed: %d ms", t.elapsed());
  Region parsedRegion(region.begin(),
                      endChildIndex >= 0 ? children[endChildIndex].region.end() : region.end());
  return std::make_tuple(nodes, parsedRegion);
}

QString LanguageParser::getData(int a, int b) {
  const auto& txt = text();
  a = clamp(0, txt.length(), a);
  b = clamp(0, txt.length(), b);
  return txt.mid(a, b - a);
}

QString LanguageParser::text() {
  return m_text;
}

void LanguageParser::setText(const QString& text) {
  m_text = text;
}

void LanguageParser::clearCache() {
  if (m_lang) {
    m_lang->clearCache();
  }
}

int LanguageParser::beginOfLine(int pos) {
  const auto& txt = text();
  if (pos < 0 || txt.size() - 1 < pos) {
    return -1;
  }

  Q_ASSERT(0 <= pos && pos <= txt.size() - 1);

  if (txt[pos] == '\n') {
    pos--;
  }

  while (pos >= 0 && txt[pos] != '\n') {
    pos--;
  }

  // begin of the line is 0 or the next char of '\n'
  pos++;

  return pos;
}

// When QTextDocument#setPlainText is called, two contentsChange events are fired. But first one has
// wrong charsRemoved and charsAdded (actual charsRemoved + 1 and actual charsAdded + 1
// respectively), so we need to workaround it in this method.
// contentsChange(pos: 0, charsRemoved: 6716, charsAdded: 0)
// contentsChange(pos: 0, charsRemoved: 0, charsAdded: 6715)
// https://bugreports.qt.io/browse/QTBUG-3495
int LanguageParser::endOfLine(int pos) {
  const auto& txt = text();

  if (pos < 0) {
    return -1;
  }

  while (pos < txt.size() && txt[pos] != '\n') {
    pos++;
  }

  if (txt.size() <= pos) {
    return txt.size() - 1;
  }

  return pos;
}

bool LanguageParser::isIdle() {
  return m_state == State::Idle;
}

void LanguageParser::setState(LanguageParser::State state) {
  m_state = state;
}

// Thread safe
bool LanguageParser::isFullParsing() {
  return m_state == State::FullParsing;
}

bool LanguageParser::isParsing() {
  return m_state == State::FullParsing || m_state == State::PartialParsing;
}

void LanguageParser::cancel() {
  setState(State::CancelRequested);
}

bool LanguageParser::isCancelRequested() {
  return m_state == State::CancelRequested;
}

LanguageParser::LanguageParser() : m_state(State::Idle) {}

LanguageParser::LanguageParser(std::unique_ptr<Language> lang, const QString& str)
    : m_lang(std::move(lang)), m_state(State::Idle) {
  setText(str);
}

Node::Node(const QString& p_name) : name(p_name) {}

Node::Node(const QString& p_name, Region p_region) : region(p_region), name(p_name) {}

void Node::append(const Node& child) {
  // skip empty region
  if (!child.region.isEmpty()) {
    children.push_back(child);
  }
}

void Node::appendTmp(Node* child) {
  // skip empty region
  if (!child->region.isEmpty()) {
    tmpChildren.push_back(child);
  }
}

void Node::moveTmpChildren() {
  for (auto child : tmpChildren) {
    child->moveTmpChildren();
    children.append(*child);
  }
  tmpChildren.clear();
}

Region Node::updateRegion() {
  for (auto& child : children) {
    Region curr = child.updateRegion();
    if (curr.begin() < region.begin()) {
      region.setBegin(curr.begin());
    }
    if (curr.end() > region.end()) {
      region.setEnd(curr.end());
    }
  }
  return region;
}

QString Node::toString(const QString& text) const {
  return format("", text);
}

bool Node::isLeaf() const {
  return children.size() == 0;
}

void Node::adjust(int pos, int delta) {
  region.adjust(pos, delta);
  for (auto& child : children) {
    child.adjust(pos, delta);
  }
}

// Remove children that intersect region
void Node::removeChildren(Region region) {
  for (auto it = children.begin(); it != children.end();) {
    if ((*it).region.intersects(region)) {
      it = children.erase(it);
    } else {
      it++;
    }
  }
}

void Node::addChildren(QList<Node> newNodes) {
  std::for_each(newNodes.begin(), newNodes.end(),
                [&](const Node& node) { children.push_back(node); });
}

void Node::sortChildren() {
  std::sort(children.begin(), children.end(),
            [](const Node& x, const Node& y) { return x.region.begin() < y.region.begin(); });
}

QString Node::format(QString indent, const QString& text) const {
  if (isLeaf()) {
    return indent +
           QStringLiteral("%1-%2: \"%3\" - Data: \"%4\"\n")
               .arg(region.begin())
               .arg(region.end())
               .arg(name)
               .arg(data(text));
  }
  QString ret =
      indent + QStringLiteral("%1-%2: \"%3\"\n").arg(region.begin()).arg(region.end()).arg(name);
  indent += "  ";

  for (auto& child : children) {
    ret = ret + child.format(indent, text);
  }
  return ret;
}

QString Node::data(const QString& text) const {
  int a = region.begin();
  int b = region.end();
  a = clamp(0, text.length(), a);
  b = clamp(0, text.length(), b);
  return text.mid(a, b - a);
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

// find within [beginPos, endPos)
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

Pattern::Pattern(Language* lang, Pattern* parent) : lang(lang), parent(parent) {}

std::pair<Pattern*, boost::optional<QVector<Region>>> Pattern::searchInPatterns(const QString& str,
                                                                                int beginPos,
                                                                                int endPos) {
  //  qDebug("firstMatch. pos: %d", pos);
  int startIdx = -1;
  Pattern* resultPattern = nullptr;
  boost::optional<QVector<Region>> resultRegions;
  int i = 0;
  // todo: think about better cache not to use this
  QVector<Pattern*> backslashGPatterns;

  while (i < cachedPatterns.length()) {
    auto pair = cachedPatterns[i]->find(str, beginPos, endPos);
    Pattern* pattern = pair.first;
    boost::optional<QVector<Region>> regions = pair.second;

    if (regions && regions->size() > 0) {
      // todo: consider the case when the pattern matches more than two lines
      auto multilineMatchedRegion = (*regions)[0];
      int newlineIndex = str.midRef(multilineMatchedRegion.begin(),
                                    multilineMatchedRegion.end() - multilineMatchedRegion.begin())
                             .indexOf('\n');
      if (0 <= newlineIndex &&
          multilineMatchedRegion.begin() + newlineIndex < multilineMatchedRegion.end() - 1) {
        //        qDebug() << "Regex matches multi line";
        // Regex matches multi line. Force to match it against single line.
        int newlinePos = multilineMatchedRegion.begin() + newlineIndex;

        // find within [multilineMatchedRegion.begin(), newlinePos]
        for (int beginPosInLine = multilineMatchedRegion.begin(); beginPosInLine < newlinePos;
             beginPosInLine++) {
          cachedPatterns[i]->clearCache();
          pair = cachedPatterns[i]->find(str, beginPosInLine, newlinePos + 1);
          pattern = pair.first;
          regions = pair.second;
          if (regions) {
            break;
          }
        }

        // If it's not found in previous line, find in next line [newlinePos + 1,
        // multilineMatchedRegion.end())
        if (!regions) {
          for (int beginPosInLine = newlinePos + 1; beginPosInLine < multilineMatchedRegion.end();
               beginPosInLine++) {
            cachedPatterns[i]->clearCache();
            pair = cachedPatterns[i]->find(str, beginPosInLine, multilineMatchedRegion.end());
            pattern = pair.first;
            regions = pair.second;
            if (regions) {
              break;
            }
          }
        }
      }
    }

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
      if (cachedPatterns[i]->match && cachedPatterns[i]->match->pattern().contains(R"(\G)")) {
        backslashGPatterns.append(cachedPatterns[i]);
        cachedPatterns[i]->clearCache();
      }

      cachedPatterns.removeAt(i);
    }
  }

  for (auto p : backslashGPatterns) {
    cachedPatterns.prepend(p);
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
                                                                    int beginPos,
                                                                    int endPos) {
  //  qDebug(" pos: %d. data.size: %d", pos, data.size());
  int actualEndPos = endPos == -1 ? str.length() : endPos;

  if (!cachedStr.isEmpty() && cachedStr == str) {
    if (!cachedResultRegions) {
      //      qDebug("cachedMatch is null");
      return std::make_pair(nullptr, boost::none);
    }

    if ((*cachedResultRegions)[0].begin() >= beginPos &&
        (*cachedResultRegions)[0].end() <= actualEndPos &&
        cachedResultPattern.loadAcquire()->cachedResultRegions) {
      //      qDebug("hits++");
      //      hits++;
      return std::make_pair(cachedResultPattern.loadAcquire(), cachedResultRegions);
    }
  } else {
    //    qDebug("cachedPatterns = nullptr");
    cachedPatterns.clear();
  }

  if (cachedPatterns.isEmpty()) {
    cachedPatterns = QVector<Pattern*>(patterns ? patterns->size() : 0);
    //    qDebug("copying patterns to cachedPatterns. cachedPatterns.size: %d",
    //    cachedPatterns->size());
    for (int i = 0; i < cachedPatterns.size(); i++) {
      cachedPatterns[i] = (*patterns)[i];
    }

    if (patterns) {
      Q_ASSERT(cachedPatterns.size() == patterns->size());
    } else {
      Q_ASSERT(cachedPatterns.size() == 0);
    }
  }
  //  qDebug("misses++");
  //  misses++;

  Pattern* pattern = nullptr;
  boost::optional<QVector<Region>> regions;
  if (match) {
    pattern = this;
    regions = match->find(str, beginPos, endPos);
  } else if (begin) {
    pattern = this;
    regions = begin->find(str, beginPos, endPos);
  } else if (!include.isEmpty()) {
    // # means an item name in the repository
    if (include.startsWith('#')) {
      QString key = include.mid(1);
      if (auto p2 = findInRepository(this, key)) {
        //        qDebug("include %s", qPrintable(include));
        auto pair = p2->find(str, beginPos, endPos);
        pattern = pair.first;
        regions = pair.second;
      } else {
        qWarning() << "Not found in repository:" << include;
      }
      // $self means the current syntax definition
    } else if (include == "$self") {
      return lang->rootPattern->find(str, beginPos, endPos);
      // $base equals $self if it doesn't have a parent. When it does, $base means parent syntax
      // e.g. When source.c++ includes source.c, "include $base" in source.c means including
      // source.c++
    } else if (include == "$base" && lang->baseLanguage) {
      return lang->baseLanguage->rootPattern->find(str, beginPos, endPos);
    } else if (cachedIncludedLanguage) {
      return cachedIncludedLanguage->rootPattern->find(str, beginPos, endPos);
      // external syntax definitions e.g. source.c++
    } else if (auto includedLang = LanguageProvider::languageFromScope(include)) {
      cachedIncludedLanguage.reset(includedLang);
      cachedIncludedLanguage->baseLanguage = lang;
      return cachedIncludedLanguage->rootPattern->find(str, beginPos, endPos);
    } else {
      qWarning() << "Include directive " + include + " failed";
    }
  } else {
    auto pair = searchInPatterns(str, beginPos, endPos);
    pattern = pair.first;
    regions = pair.second;
  }

  cachedStr = QStringRef(&str);
  cachedResultRegions = regions;
  cachedResultPattern.storeRelease(pattern);

  return std::make_pair(pattern, regions);
}

Node Pattern::createNode(const QString& str, const QVector<Region>& regions) {
  Q_ASSERT(!regions.isEmpty());

  //  qDebug() << "createNode. mo:" << *mo;

  Node node(name, regions[0]);

  if (match) {
    createCaptureNodes(regions, &node, captures);
  }

  if (!begin) {
    node.updateRegion();
    return node;
  }

  if (beginCaptures.length() > 0) {
    createCaptureNodes(regions, &node, beginCaptures);
  } else {
    createCaptureNodes(regions, &node, captures);
  }

  if (!end) {
    node.updateRegion();
    return node;
  }

  bool found = false;
  int i, endPos;

  // Store cached result regions because searchInPatterns may overwrite it.
  // If it's overwritten, next iteration in for loop has different result regions compared by
  // previous iteration
  // Don't cache cachedPatterns. It's supposed to be overwritten in searchInPatterns.
  auto tmpCachedRegions = cachedResultRegions;

  for (i = node.region.end(), endPos = str.length(); i < str.length();) {
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
    if (!cachedPatterns.isEmpty()) {
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
            node.region.isEmpty()))) {
        found = true;
        Node r = patternBeforeEnd->createNode(str, *regionsBeforeEnd);
        i = r.region.end();

        // If r->region is empty, it leads infinite loop without i++;
        if (r.region.isEmpty()) {
          i++;
        }
        node.append(r);

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
      Node newNode(contentName, Region(node.region.end(), (*endMatchedRegions)[0].begin()));
      node.append(newNode);
    }

    if (endCaptures.length() > 0) {
      createCaptureNodes(*endMatchedRegions, &node, endCaptures);
    } else {
      createCaptureNodes(*endMatchedRegions, &node, captures);
    }

    break;
  }

  node.region.setEnd(endPos);
  node.updateRegion();
  return node;
}

void Pattern::createCaptureNodes(QVector<Region> regions, Node* parent, Captures captures) {
  QVector<int> parentIndices(regions.length());
  QVector<Node*> parents(parentIndices.length());

  // This exists to store pointers of children created in this method because we keep track of the
  // pointer of child in tmpChildren.
  QList<std::shared_ptr<Node>> newChildren;

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
    std::shared_ptr<Node> child(new Node(v.name, regions[i]));
    newChildren.append(child);

    parents[i] = child.get();

    if (i == 0) {
      parent->appendTmp(child.get());
      continue;
    }

    Node* p = nullptr;
    while (!p) {
      i = parentIndices[i];
      p = parents[i];
    }
    p->appendTmp(child.get());
  }

  // Copy the content of shared_ptr from tmpChildren to actual children
  parent->moveTmpChildren();
}

void Pattern::clearCache() {
  cachedStr.clear();
  cachedResultPattern.storeRelease(nullptr);
  cachedPatterns.clear();
  cachedResultRegions = boost::none;
  cachedIncludedLanguage.reset(nullptr);
  if (patterns) {
    foreach (Pattern* pat, *patterns) { pat->clearCache(); }
  }
  for (auto& pair : repository) {
    Q_ASSERT(pair.second);
    pair.second->clearCache();
  }
}

QVector<QPair<QString, QString>> LanguageProvider::s_scopeAndLangNamePairs(0);
QMap<QString, QString> LanguageProvider::s_scopeLangFilePathMap;
QMap<QString, QString> LanguageProvider::s_extensionLangFilePathMap;
QReadWriteLock LanguageProvider::s_lock;

Language* LanguageProvider::defaultLanguage() {
  return languageFromScope(DEFAULT_SCOPE);
}

Language* LanguageProvider::languageFromScope(const QString& scope) {
  QReadLocker locker(&s_lock);

  if (s_scopeLangFilePathMap.contains(scope)) {
    locker.unlock();
    return loadLanguage(s_scopeLangFilePathMap.value(scope));
  } else {
    return nullptr;
  }
}

Language* LanguageProvider::languageFromExtension(const QString& ext) {
  QReadLocker locker(&s_lock);

  if (s_extensionLangFilePathMap.contains(ext)) {
    locker.unlock();
    return loadLanguage(s_extensionLangFilePathMap.value(ext));
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

  QVariantMap rootMap = root.toMap();
  Language* lang = new Language(rootMap);

  QWriteLocker locker(&s_lock);

  if (!s_scopeLangFilePathMap.contains(lang->scopeName)) {
    foreach (const QString& ext, lang->fileTypes) { s_extensionLangFilePathMap[ext] = path; }
    s_scopeLangFilePathMap[lang->scopeName] = path;
    s_scopeAndLangNamePairs.append(QPair<QString, QString>(lang->scopeName, lang->name()));
  }

  return lang;
}

QVector<QPair<QString, QString>> LanguageProvider::scopeAndLangNamePairs() {
  QReadLocker locker(&s_lock);

  return s_scopeAndLangNamePairs;
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

Language::Language(QVariantMap rootMap)
    : rootPattern(nullptr), baseLanguage(this), hideFromUser(false) {
  // fileTypes
  if (rootMap.contains(FILE_TYPES_KEY)) {
    QVariant fileTypesVar = rootMap.value(FILE_TYPES_KEY);
    if (fileTypesVar.canConvert<QVariantList>()) {
      QSequentialIterable iterable = fileTypesVar.value<QSequentialIterable>();
      foreach (const QVariant& v, iterable) {
        if (v.canConvert<QString>()) {
          QString ext = v.toString();
          fileTypes.append(ext);
        }
      }
    }
  }

  // hideFromUser
  if (rootMap.contains(HIDE_FROM_USER_KEY)) {
    QVariant var = rootMap.value(HIDE_FROM_USER_KEY);
    if (var.canConvert<bool>()) {
      hideFromUser = var.toBool();
    }
  }

  // firstLineMatch
  if (rootMap.contains(FIRST_LINE_MATCH_KEY)) {
    firstLineMatch = rootMap.value(FIRST_LINE_MATCH_KEY).toString();
  }

  // scopeName
  if (rootMap.contains(SCOPE_NAME_KEY)) {
    scopeName = rootMap.value(SCOPE_NAME_KEY).toString();
  }

  // patterns
  rootPattern.reset(toRootPattern(rootMap, this));
}

QString Language::name() {
  return rootPattern ? rootPattern->name : "";
}

void Language::clearCache() {
  if (rootPattern) {
    rootPattern->clearCache();
  }
}

RootNode::RootNode() : Node() {}

RootNode::RootNode(const QString& name) : Node(name) {}

void RootNode::adjust(int pos, int delta) {
  region.setEnd(region.end() + delta);
  for (auto& child : children) {
    child.adjust(pos, delta);
  }
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
