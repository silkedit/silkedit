#include <memory>
#include <QTabWidget>
#include <QTabBar>
#include <QPlainTextDocumentLayout>
#include <QFileInfo>

#include "LayoutView.h"
#include "TextEditView.h"
#include "FileDocument.h"
#include "STabWidget.h"

LayoutView::LayoutView() : m_tabbar(new STabWidget(this)), m_layout(new QHBoxLayout) {
  m_tabbar->setAcceptDrops(true);

  m_tabbar->setElideMode(Qt::ElideRight);
  //    m_tabbar->setSelectionBehaviorOnRemove (QTabBar::SelectLeftTab);

  m_tabbar->setMovable(true);
  m_tabbar->setDocumentMode(true);
  m_tabbar->setTabsClosable(true);

  m_layout->addWidget(m_tabbar.get());
  m_layout->setSpacing(0);
  m_layout->setMargin(0);
  // LayoutView takes ownership of this layout by calling setLayout
  setLayout(m_layout);
  setContentsMargins(0, 0, 0, 0);

  std::unique_ptr<TextEditView> view(new TextEditView);
  m_tabbar->addTab(std::move(view), "untitled");
}

void LayoutView::addDocument(const QString& filename, QTextDocument* doc) {
  std::unique_ptr<TextEditView> view(new TextEditView);
  view->setDocument(doc);
  QFileInfo info(filename);
  m_tabbar->addTab(std::move(view), info.fileName());
}
