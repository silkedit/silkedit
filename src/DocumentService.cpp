#include <memory>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <stextedit.h>

#include "DocumentService.h"
#include "LayoutView.h"

bool DocumentService::open(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadWrite))
    return false;

  QTextStream in(&file);
  std::unique_ptr<QTextDocument> doc(new QTextDocument(in.readAll()));
  STextDocumentLayout* layout = new STextDocumentLayout(doc.get());
  doc->setDocumentLayout(layout);

  if (m_layoutView) {
    m_layoutView->addDocument(file.fileName(), doc.get());
  } else {
    qWarning("m_layoutView is null");
  }

  m_documents.insert(std::make_pair(filename, std::move(FileDocument(filename, std::move(doc)))));
  return true;
}

DocumentService::DocumentService() : m_layoutView(nullptr) {
}
