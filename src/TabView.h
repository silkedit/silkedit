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
  int insertTab(int index, QWidget* w, const QString& label);
  TextEditView* activeEditView() { return m_activeEditView; }
  bool tabDragging() { return m_tabDragging; }
  void saveAllTabs();
  int indexOfPath(const QString& path);
  int insertTabInformation( const int index );
  bool createWithSavedTabs();

 public slots:
  void closeActiveTab();
  bool closeAllTabs();
  void closeOtherTabs();
  int open(const QString& path);
  void addNew();

 signals:
  void allTabRemoved();
  void activeTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);

 protected:
  void tabInserted(int index) override;
  void tabRemoved(int index) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  TextEditView* m_activeEditView;
  TabBar* m_tabBar;
  bool m_tabDragging;

  void setActiveEditView(TextEditView* editView);
  void removeTabAndWidget(int index);
  bool closeTab(QWidget* w);
  void focusTabContent(int index);
  void updateTabTextBasedOn(bool changed);
  void changeActiveEditView(int index);
  void changeTabText(const QString& path);
  void changeTabStyle(core::Theme* theme);
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint& newWindowPos, bool isFloating);
};

Q_DECLARE_METATYPE(TabView*)
