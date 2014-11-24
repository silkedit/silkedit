#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <memory>

#include "macros.h"
#include "STabWidget.h"

class QString;
class QTextDocument;

class LayoutView : public QWidget {
  DISABLE_COPY(LayoutView)
 public:
  LayoutView();
  ~LayoutView() = default;
  DEFAULT_MOVE(LayoutView)

  void addDocument(const QString& filename, QTextDocument* doc);

  std::unique_ptr<STabWidget> m_tabbar;
  QHBoxLayout* m_layout;
};
