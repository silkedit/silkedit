#pragma once

#include <QUuid>

namespace core {

class UniqueObject {
 public:
  UniqueObject() = default;

  QUuid id();

 protected:
  virtual ~UniqueObject();

 private:
  QUuid m_id;
};

}  // namespace core
