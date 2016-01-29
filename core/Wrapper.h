#pragma once

#include <QObject>
#include <QVariant>

#define WRAPPED "wrapped"
#define WRAPPED_METHOD_SIGNATURE "getWrapped()"

namespace core {

class Wrapper : public QObject {
  Q_OBJECT

public slots:
  QVariant getWrapped() { return m_wrapped; }

 protected:
  QVariant m_wrapped;
};

}  // namespace core
