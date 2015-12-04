#include "BOM.h"
#include <QDebug>
#include <QTextCodec>

namespace core {

const BOM BOM::getBOM(BOMSwitch sw) {
  static BOM s_def[BOMSwitch::BOMSwitchSize] = {
      BOM(QObject::tr("BOM"), QObject::tr("BOM"), true),
      BOM(QObject::tr("No BOM"), QObject::tr("No BOM"), false),
  };
  switch (sw) {
    default:
    case BOMSwitch::On:
      return s_def[BOMSwitch::On];
      break;
    case BOMSwitch::Off:
      return s_def[BOMSwitch::Off];
      break;
  }
}

const BOM BOM::guessBOM(const QByteArray& array) {
  bool bom = QTextCodec::codecForUtfText(array.mid(0,4), nullptr) != nullptr;
  if(bom) {
    qDebug() << "Detect BOM.";
    return getBOM(BOMSwitch::On);
  }else{
    qDebug() << "Not Detect BOM.";
    return getBOM(BOMSwitch::Off);
  }
}

const BOM BOM::defaultBOM() {
  return getBOM(BOMSwitch::On);
}

const boost::optional<BOM> BOM::bomForName(const QString& name) {
  const BOM& On = getBOM(BOMSwitch::On);
  const BOM& Off = getBOM(BOMSwitch::Off);
  if (On.name() == name) {
    return On;
  } else if (Off.name() == name) {
    return Off;
  }
  return boost::none;
}

BOM::BOM(const QString& name, const QString& displayName, bool bomSwitch)
    : m_name(name), m_displayName(displayName), m_bomSwitch(bomSwitch) {}

bool BOM::operator==(const BOM& other) const {
  return this->name() == other.name();
}

bool BOM::operator!=(const BOM& other) const {
  return !(*this == other);
}

}  // namespace core
