#include <string>
#include <QDebug>
#include <QVariant>

#include "Config.h"
#include "ThemeManager.h"
#include "V8Util.h"
#include "ObjectStore.h"
#include "Font.h"
#include "atom/node_includes.h"

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::PropertyCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::MaybeLocal;
using v8::ObjectTemplate;
using v8::Maybe;

namespace {
const QString END_OF_LINE_STR_KEY = QStringLiteral("end_of_line_str");
const QString END_OF_FILE_STR_KEY = QStringLiteral("end_of_file_str");
const QString THEME_KEY = QStringLiteral("theme");
const QString FONT_FAMILY_KEY = QStringLiteral("font_family");
const QString FONT_SIZE_KEY = QStringLiteral("font_size");
const QString INDENT_USING_SPACES_KEY = QStringLiteral("indent_using_spaces");
const QString TAB_WIDTH_KEY = QStringLiteral("tab_width");
const QString LOCALE_KEY = QStringLiteral("locale");
const QString SHOW_INVISIBLES_KEY = QStringLiteral("show_invisibles");
const QString SHOW_TABS_AND_SPACES_KEY = QStringLiteral("show_tabs_and_spaces");
const QString WORD_WRAP_KEY = QStringLiteral("word_wrap");

QHash<QString, QVariant::Type> keyTypeHashForBuiltinConfigs;

void initKeyTypeHash() {
  keyTypeHashForBuiltinConfigs[END_OF_LINE_STR_KEY] = QVariant::String;
  keyTypeHashForBuiltinConfigs[END_OF_FILE_STR_KEY] = QVariant::String;
  keyTypeHashForBuiltinConfigs[THEME_KEY] = QVariant::String;
  keyTypeHashForBuiltinConfigs[FONT_FAMILY_KEY] = QVariant::String;
  keyTypeHashForBuiltinConfigs[FONT_SIZE_KEY] = QVariant::Int;
  keyTypeHashForBuiltinConfigs[INDENT_USING_SPACES_KEY] = QVariant::Bool;
  keyTypeHashForBuiltinConfigs[TAB_WIDTH_KEY] = QVariant::Int;
  keyTypeHashForBuiltinConfigs[LOCALE_KEY] = QVariant::String;
  keyTypeHashForBuiltinConfigs[SHOW_INVISIBLES_KEY] = QVariant::Bool;
  keyTypeHashForBuiltinConfigs[SHOW_TABS_AND_SPACES_KEY] = QVariant::Bool;
  keyTypeHashForBuiltinConfigs[WORD_WRAP_KEY] = QVariant::Bool;
}
}

namespace core {

void Config::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<ObjectTemplate> objTempl = ObjectTemplate::New(isolate);
  objTempl->SetInternalFieldCount(1);
  MaybeLocal<Object> maybeObj = objTempl->NewInstance(isolate->GetCurrentContext());
  if (maybeObj.IsEmpty()) {
    throw std::runtime_error("Failed to create Config");
  }

  Local<Object> obj = maybeObj.ToLocalChecked();
  obj->SetAlignedPointerInInternalField(0, &Config::singleton());

  NODE_SET_METHOD(obj, "get", get);
  NODE_SET_METHOD(obj, "set", set);
  NODE_SET_METHOD(obj, "setFont", setFont);

  Maybe<bool> result =
      exports->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "Config"), obj);
  if (result.IsNothing()) {
    throw std::runtime_error("setting exports failed");
  }
}

void Config::setTheme(Theme* theme, bool noSave) {
  if (m_theme != theme) {
    m_theme = theme;
    m_scalarConfigs[THEME_KEY] = QVariant(theme->name);
    if (!noSave) {
      save(THEME_KEY, theme->name);
    }
    emit themeChanged(theme);
  }
}

void Config::setFont(const QFont& font) {
  if (m_font != font) {
    m_font = font;
    // http://doc.qt.io/qt-5.5/qfont.html#HintingPreference-enum
    // On Windows, FullHinting makes some fonts ugly
    // On Mac, hintingPreference is ignored.
    m_font.setHintingPreference(QFont::PreferVerticalHinting);
    m_scalarConfigs[FONT_FAMILY_KEY] = QVariant(font.family());
    m_scalarConfigs[FONT_SIZE_KEY] = QVariant(font.pointSize());
    save(FONT_FAMILY_KEY, font.family());
    save(FONT_SIZE_KEY, font.pointSize());
    emit fontChanged(m_font);
  }
}

int Config::tabWidth() {
  return get(TAB_WIDTH_KEY, 4);
}

void Config::setTabWidth(int tabWidth) {
  if (setValue(TAB_WIDTH_KEY, tabWidth)) {
    emit tabWidthChanged(tabWidth);
  }
}

bool Config::indentUsingSpaces() {
  return get(INDENT_USING_SPACES_KEY, false);
}

void Config::setIndentUsingSpaces(bool value) {
  if (setValue(INDENT_USING_SPACES_KEY, value)) {
    emit indentUsingSpacesChanged(value);
  }
}

void Config::init() {
  m_mapConfigs.clear();
  m_scalarConfigs.clear();

  initKeyTypeHash();

  load();

  setTheme(ThemeManager::theme(themeName()));
  QFont font(fontFamily(), fontSize());
  setFont(font);
}

std::unordered_map<std::string, std::string> Config::mapValue(const QString& key) {
  if (m_mapConfigs.count(key) != 0) {
    return m_mapConfigs[key];
  }

  return std::unordered_map<std::string, std::string>();
}

bool Config::contains(const QString& key) {
  return m_scalarConfigs.count(key) != 0;
}

void Config::addPackageConfigDefinition(const ConfigDefinition& def) {
  m_packageConfigDefinitions[def.key] = def;
}

void Config::emitConfigChange(const QString& key, QVariant value) {
  if (key == SHOW_TABS_AND_SPACES_KEY && value.canConvert<bool>()) {
    emit showTabsAndSpacesChanged(value.toBool());
  } else if (key == WORD_WRAP_KEY && value.canConvert<bool>()) {
    emit wordWrapChanged(value.toBool());
  }
}

QVariant convert(QVariant var, QVariant::Type type) {
  Q_ASSERT(var.type() == QVariant::Type::ByteArray);

  switch (type) {
    case QVariant::Type::String:
      return QString::fromUtf8(var.toByteArray());
    case QVariant::Type::Bool:
      if (var.toByteArray() == "true") {
        return QVariant::fromValue(true);
      } else if (var.toByteArray() == "false") {
        return QVariant::fromValue(false);
      } else {
        return QVariant();
      }
    case QVariant::Type::Int: {
      bool ok;
      int value = var.toByteArray().toInt(&ok);
      return ok ? QVariant::fromValue(value) : QVariant();
    }
    case QVariant::Type::Double: {
      bool ok;
      double value = var.toByteArray().toDouble(&ok);
      return ok ? QVariant::fromValue(value) : QVariant();
    }
    default:
      qWarning() << "invalid type:" << type;
      return QVariant();
  }
}

QVariant Config::get(const QString& key) {
  if (m_scalarConfigs.count(key) != 0) {
    QVariant var = m_scalarConfigs.at(key);
    // Configs defined in packages are stored as byte array (because we don't know their type when
    // parsing config.yml), so convert it to appropriate type defined in config_definitinos.yml
    if (var.type() == QVariant::Type::ByteArray && m_packageConfigDefinitions.contains(key)) {
      QVariant newValue = convert(var, m_packageConfigDefinitions.value(key).type());
      m_scalarConfigs[key] = newValue;
    }
    return m_scalarConfigs[key];
  }

  // check default value for package config
  if (m_packageConfigDefinitions.contains(key)) {
    return m_packageConfigDefinitions.value(key).defaultValue;
  }

  return QVariant();
}

void Config::set(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 2 || !args[0]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  auto newValue = V8Util::toVariant(isolate, args[1]);

  Config::singleton().setValue(key, newValue);
}

void Config::get(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1 || !args[0]->IsString()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  const auto& key =
      V8Util::toQString(args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  QVariant value = Config::singleton().get(key);
  if (!value.isValid()) {
    std::stringstream ss;
    ss << "config: " << key.toUtf8().constData() << " not found";
    V8Util::throwError(isolate, ss.str());
    return;
  }

  args.GetReturnValue().Set(V8Util::toV8Value(isolate, value));
}

void Config::setFont(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1 || !args[0]->IsObject()) {
    V8Util::throwError(isolate, "invalid argument");
    return;
  }

  Local<Object> obj = args[0]->ToObject();
  if (obj->InternalFieldCount() > 0) {
    QObject* fontObj = ObjectStore::unwrap(obj);
    if (!fontObj) {
      V8Util::throwError(isolate, "QObject is null");
      return;
    }

    Font* font = qobject_cast<Font*>(fontObj);
    if (!font) {
      V8Util::throwError(isolate, "QObject is not Font");
      return;
    }

    Config::singleton().setFont(font->getWrapped().value<QFont>());
  } else {
    V8Util::throwError(isolate, "can't find the internal field");
    return;
  }
}

QString Config::endOfLineStr() {
  return get(END_OF_LINE_STR_KEY, u8"\u00AC");  // U+00AC is '¬'
}

void Config::setEndOfLineStr(const QString& newValue) {
  if (setValue(END_OF_LINE_STR_KEY, newValue)) {
    emit endOfLineStrChanged(newValue);
  }
}

QString Config::endOfFileStr() {
  return get(END_OF_FILE_STR_KEY, "");
}

bool Config::enableMnemonic() {
  return get("enable_mnemonic", false);
}

QString Config::locale() {
  const QString& systemLocale = QLocale::system().name();
  const QString& locale = get(LOCALE_KEY, systemLocale);
  if (locale == "system") {
    return systemLocale;
  }
  return locale;
}

void Config::setLocale(const QString& newValue) {
  setValue(LOCALE_KEY, newValue);
}

bool Config::showInvisibles() {
  return get(SHOW_INVISIBLES_KEY, false);
}

void Config::setShowInvisibles(bool newValue) {
  if (setValue(SHOW_INVISIBLES_KEY, newValue)) {
    emit showInvisiblesChanged(newValue);
  }
}

bool Config::showTabsAndSpaces() {
  return get(SHOW_TABS_AND_SPACES_KEY, defaultValue(SHOW_TABS_AND_SPACES_KEY).toBool());
}

bool Config::wordWrap() {
  return get(WORD_WRAP_KEY, defaultValue(WORD_WRAP_KEY).toBool());
}

Config::Config() : m_theme(nullptr) {}

void Config::load() {
  QStringList existingConfigPaths;
  foreach (const QString& path, Constants::singleton().configPaths()) {
    if (QFile(path).exists()) {
      existingConfigPaths.append(path);
    }
  }

  foreach (const QString& path, existingConfigPaths) { load(path); }
}

void Config::load(const QString& filename) {
  qDebug("loading configuration");

  if (!QFile(filename).exists())
    return;

  std::string name = filename.toUtf8().constData();
  try {
    YAML::Node rootNode = YAML::LoadFile(name);

    if (!rootNode.IsMap()) {
      throw std::runtime_error("root node must be a map");
    }

    for (auto it = rootNode.begin(); it != rootNode.end(); ++it) {
      QString key = QString::fromUtf8(it->first.as<std::string>().c_str()).trimmed();
      if (it->second.IsScalar()) {
        // If key is predefined, save it as an appropriate type.
        if (keyTypeHashForBuiltinConfigs.contains(key)) {
          switch (keyTypeHashForBuiltinConfigs[key]) {
            case QVariant::Bool:
              m_scalarConfigs[key] = QVariant(it->second.as<bool>());
              break;
            case QVariant::Int:
              m_scalarConfigs[key] = QVariant(it->second.as<int>());
              break;
            case QVariant::String: {
              QString value = QString::fromUtf8(it->second.as<std::string>().c_str()).trimmed();
              m_scalarConfigs[key] = value;
              break;
            }
            default:
              qWarning("Invalid type %d. key: %s", keyTypeHashForBuiltinConfigs[key],
                       qPrintable(key));
              break;
          }
          // If key is defined in a package, save it as byte array
        } else {
          QByteArray value = it->second.as<std::string>().c_str();
          m_scalarConfigs[key] = value;
        }
      } else if (it->second.IsMap()) {
        YAML::Node mapNode = it->second;
        std::unordered_map<std::string, std::string> map;
        for (auto mapIt = mapNode.begin(); mapIt != mapNode.end(); mapIt++) {
          if (mapIt->first.IsScalar() && mapIt->second.IsScalar()) {
            std::string mapKey = mapIt->first.as<std::string>();
            std::string mapValue = mapIt->second.as<std::string>();
            map.insert(std::make_pair(mapKey, mapValue));
          }
        }
        m_mapConfigs[key] = map;
      }
    }
  } catch (const std::exception& e) {
    qWarning() << "can't load yaml file:" << filename << ", reason: " << e.what();
  } catch (...) {
    qWarning() << "can't load yaml file because of an unexpected exception: " << filename;
  }
}

QString Config::themeName() {
  return get(THEME_KEY, "Solarized (light)");
}

QString Config::fontFamily() {
  return get(FONT_FAMILY_KEY, Constants::singleton().defaultFontFamily);
}

int Config::fontSize() {
  return get(FONT_SIZE_KEY, Constants::singleton().defaultFontSize);
}

QVariant Config::defaultValue(const QString& key) {
  if (key == SHOW_TABS_AND_SPACES_KEY) {
    return QVariant::fromValue(false);
  } else if (key == WORD_WRAP_KEY) {
    return QVariant::fromValue(true);
  } else {
    return QVariant();
  }
}

}  // namespace core
