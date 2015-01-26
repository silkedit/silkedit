#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>

#include "macros.h"
#include "set_unique_ptr.h"

class TextEditView;
class TabBar;

class TabView : public QTabWidget {
  Q_OBJECT
  DISABLE_COPY(TabView)

 public:
  explicit TabView(QWidget* parent = nullptr);
  ~TabView();
  DEFAULT_MOVE(TabView)

  int addTab(QWidget* page, const QString& label);
  int insertTab(int index, QWidget* w, const QString& label);
  int open(const QString& path);
  void addNew();
  TextEditView* activeEditView() { return m_activeEditView; }
  bool tabDragging() { return m_tabDragging; }
  void saveAllTabs();
  void closeActiveTab();
  bool closeAllTabs();
  void closeOtherTabs();
  int indexOfPath(const QString& path);

signals:
  void allTabRemoved(bool afterDrag);
  void activeTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);

 public slots:
  // Detach Tab
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint& newWindowPos, bool isFloating);

 protected:
  void tabInserted(int index) override;
  void tabRemoved(int index) override;
  void tabRemoved(int index, bool afterDrag);
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  TextEditView* m_activeEditView;
  TabBar* m_tabBar;
  bool m_tabDragging;

  void setActiveEditView(TextEditView* editView);

  void removeTabAndWidget(int index);
  bool closeTab(QWidget* w);
  void focusTabContent(int index);

 private slots:
  void updateTabTextBasedOn(bool changed);
  void changeActiveEditView(int index);
  void changeTabText(const QString& path);
};
