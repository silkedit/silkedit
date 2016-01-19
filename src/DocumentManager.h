#pragma once

#include <QString>
#include <QObject>

#include "core/macros.h"
#include "core/Singleton.h"

class TabView;
namespace core {
class Document;
}

class DocumentManager : public QObject, public core::Singleton<DocumentManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(DocumentManager)

 public:
  static const QString DEFAULT_FILE_NAME;

  ~DocumentManager() = default;

  bool save(core::Document* doc);
  QString saveAs(core::Document* doc);

public slots:
  bool open(const QString& filename);

 private:
  friend class core::Singleton<DocumentManager>;
  DocumentManager() = default;
};
