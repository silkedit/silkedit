#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <memory>

#include "macros.h"
#include "STabWidget.h"

class QString;
class QTextDocument;
class TextEditView;

class LayoutView : public QWidget {
  DISABLE_COPY(LayoutView)
 public:
  LayoutView();
  ~LayoutView() = default;
  DEFAULT_MOVE(LayoutView)

  STabWidget* tabWidget() { return m_tabbar.get(); }

  std::unique_ptr<STabWidget> m_tabbar;
  QHBoxLayout* m_layout;
};
