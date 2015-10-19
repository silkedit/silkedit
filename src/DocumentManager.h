#pragma once

#include <QString>

#include "core/macros.h"

class TabView;
namespace core {
class Document;
}

class DocumentManager {
  DISABLE_COPY_AND_MOVE(DocumentManager)

 public:
  static const QString DEFAULT_FILE_NAME;

  static bool open(const QString& filename);
  static bool save(core::Document* doc);
  static QString saveAs(core::Document* doc);

 private:
  DocumentManager() = delete;
  ~DocumentManager() = delete;
};
