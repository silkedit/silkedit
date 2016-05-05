#pragma once

#include <string>
#include <boost/optional.hpp>
#include <QString>

#include "macros.h"

namespace core {

class BOM {
 public:
  BOM(const QString& name, const QString& displayName, bool bomSwitch);
  ~BOM() = default;
  DEFAULT_COPY_AND_MOVE(BOM)

  typedef enum {
    On = 0,
    Off,
    BOMSwitchSize,  // terminator
  } BOMSwitch;

  static const BOM getBOM(BOMSwitch sw);
  static const BOM guessBOM(const QByteArray& array);
  static const BOM defaultBOM();
  static const boost::optional<BOM> bomForName(const QString& name);

  QString name() const { return m_name; }
  QString displayName() const { return m_displayName; }
  bool bomSwitch() const { return m_bomSwitch; }

  bool operator==(const BOM& other) const;
  bool operator!=(const BOM& other) const;

 private:
  // Identify BOM type
  QString m_name;
  // Displayable BOM name
  QString m_displayName;
  // BOM on/off
  bool m_bomSwitch;
};

}  // namespace core
