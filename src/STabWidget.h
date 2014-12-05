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
  int insertTab(int index, QWidget* w, const QString& label);
  int open(const QString& path);
  void addNew();
  TextEditView* activeEditView() { return m_activeEditView; }
  bool tabDragging() { return m_tabDragging; }
  void saveAllTabs();
  void closeActiveTab();

signals:
  void allTabRemoved();

 public slots:
  // Detach Tab
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint&);

 protected:
  void tabInserted(int index) override;
  void tabRemoved(int index) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  TextEditView* m_activeEditView;
  STabBar* m_tabBar;
  bool m_tabDragging;

  void removeTabAndWidget(int index);

 private slots:
  void updateTabTextBasedOn(bool changed);
};
