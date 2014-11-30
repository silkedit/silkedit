#include <memory>
#include <QTabWidget>
#include <QTabBar>
#include <QPlainTextDocumentLayout>
#include <QFileInfo>

#include "LayoutView.h"
#include "TextEditView.h"
#include "STabWidget.h"
#include "KeymapService.h"

LayoutView::LayoutView()
    : m_tabbar(new STabWidget(this)), m_layout(new QHBoxLayout) {
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

  m_tabbar->addNew();
}
