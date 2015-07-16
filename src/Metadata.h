#include <memory>
#include <unordered_map>
#include <QString>

#include "Regexp.h"
#include "macros.h"
#include "stlSpecialization.h"

// Model class for .tmPreferences file
class Metadata {
  DISABLE_COPY(Metadata)
 public:
  static Metadata* get(const QString& scope);
  static void load(const QString& filename);

  explicit Metadata(const QString& scope);
  ~Metadata() = default;
  DEFAULT_MOVE(Metadata)

  // accessors
  QString name() { return m_name; }
  void setName(const QString& name) { m_name = name; }

  Regexp* increaseIndentPattern() { return m_increaseIndentPattern.get(); }
  void setIncreaseIndentPattern(const QString& pattern);

  Regexp* decreaseIndentPattern() { return m_decreaseIndentPattern.get(); }
  void setDecreateIndentPattern(const QString& pattern);

  Regexp* bracketIndentNextLinePattern() { return m_bracketIndentNextLinePattern.get(); }
  void setBracketIndentNextLinePattern(const QString& pattern);

  Regexp* disableIndentNextLinePattern() { return m_disableIndentNextLinePattern.get(); }
  void setDisableIndentNextLinePattern(const QString& pattern);

  Regexp* unIndentedLinePattern() { return m_unIndentedLinePattern.get(); }
  void setUnIndentedLinePattern(const QString& pattern);

 private:
  static std::unordered_map<QString, std::unique_ptr<Metadata>> s_scopeMetadataMap;

  QString m_name;
  QString m_scope;

  // Indentation Options
  // see SublimeText's site for more detail.
  // http://sublime-text-unofficial-documentation.readthedocs.org/en/latest/reference/metadata.html
  std::unique_ptr<Regexp> m_increaseIndentPattern;
  std::unique_ptr<Regexp> m_decreaseIndentPattern;
  std::unique_ptr<Regexp> m_bracketIndentNextLinePattern;
  std::unique_ptr<Regexp> m_disableIndentNextLinePattern;
  std::unique_ptr<Regexp> m_unIndentedLinePattern;
};
