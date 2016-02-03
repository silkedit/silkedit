#pragma once

#include <QString>
#include <QObject>

#include "core/macros.h"
#include "core/Singleton.h"

class ProjectManager : public QObject, public core::Singleton<ProjectManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(ProjectManager)

 public:
  ~ProjectManager() = default;

 public slots:
  bool open(const QString& directoryName);

 private:
  friend class core::Singleton<ProjectManager>;
  ProjectManager() = default;
};
