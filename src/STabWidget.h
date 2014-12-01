#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>

#include "macros.h"
#include "set_unique_ptr.h"

class TextEditView;
class STabBar;

class STabWidget : public QTabWidget {
  Q_OBJECT
  DISABLE_COPY(STabWidget)

 public:
  explicit STabWidget(QWidget* parent = nullptr);
  ~STabWidget();
  DEFAULT_MOVE(STabWidget)

  int addTab(QWidget* page, const QString& label);
  int open(const QString& path);
  void addNew();
  TextEditView* activeEditView() { return m_activeEditView; }

 public slots:
  // Detach Tab
  void DetachTab(int index, QPoint&);

 protected:
  void tabInserted(int index) override;

 private:
  std::unordered_set<set_unique_ptr<QWidget>> m_widgets;
  TextEditView* m_activeEditView;
  STabBar* m_tabBar;
};
