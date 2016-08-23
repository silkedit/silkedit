#pragma once

#include <QObject>

#include "core/macros.h"

class UpdateNotificationView : public QObject {
  Q_OBJECT
  DISABLE_COPY(UpdateNotificationView)

 public:
  static void show();

  ~UpdateNotificationView() = default;
  DEFAULT_MOVE(UpdateNotificationView)

 private:
  UpdateNotificationView() = delete;
};
