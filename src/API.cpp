#include "API.h"
#include "STabWidget.h"

void API::init(STabWidget *tabWidget)
{
  m_tabWidget = tabWidget;
}

TextEditView *API::activeEditView()
{
  if (m_tabWidget) {
    return m_tabWidget->activeEditView();
  } else {
    qWarning("m_tabWidget is null");
    return nullptr;
  }
}
