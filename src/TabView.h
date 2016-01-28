#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>
#include <QSettings>

#include "core/macros.h"
#include "core/set_unique_ptr.h"

class TextEditView;
class TabBar;
namespace core {
class Theme;
}

class TabView : public QTabWidget{
  Q_OBJECT
  DISABLE_COPY(TabView)

 public:
  explicit TabView(QWidget* parent = nullptr);
  ~TabView();
  DEFAULT_MOVE(TabView)

  int addTab(QWidget* page, const QString& label);
  int insertTab(int index, QWidget* widget, const QString& label);
  QWidget* activeView() { return m_activeView; }
  bool tabDragging() { return m_tabDragging; }
  void saveAllTabs();
  int indexOfPath(const QString& path);
  bool insertTabInformation( const int index );
  bool createWithSavedTabs();

 public slots:
  void closeActiveTab();
  bool closeAllTabs();
  void closeOtherTabs();
  int open(const QString& path);
  void addNew();

 signals:
  void allTabRemoved();
  void activeViewChanged(QWidget* oldView, QWidget* newView);

 protected:
  void tabInserted(int index) override;
  void tabRemoved(int index) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  QWidget* m_activeView;
  TabBar* m_tabBar;
  bool m_tabDragging;

  void setActiveView(QWidget *activeView);
  void removeTabAndWidget(int index);
  bool closeTab(QWidget* w);
  void focusTabContent(int index);
  void updateTabTextBasedOn(bool changed);
  void changeActiveView(int index);
  void changeTabStyle(core::Theme* theme);
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint& newWindowPos, bool isFloating);
};

Q_DECLARE_METATYPE(TabView*)
