#include <memory>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <stextedit.h>

#include "DocumentService.h"
#include "LayoutView.h"

bool DocumentService::open(const QString& filename) {
  if (m_tabWidget) {
    return m_tabWidget->open(filename) >= 0;
  } else {
    qWarning("m_tabWidget is null");
    return false;
  }
}

DocumentService::DocumentService() : m_tabWidget(nullptr) {
}
