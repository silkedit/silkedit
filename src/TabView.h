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
class Document;
}

class TabView : public QTabWidget {
  Q_OBJECT
  DISABLE_COPY(TabView)

 public:
  enum CloseTabIncludingDocResult { UserCanceled, AllTabsRemoved, Finished };

  explicit TabView(QWidget* parent = nullptr);
  ~TabView();
  DEFAULT_MOVE(TabView)

  int addTab(QWidget* page, const QString& label);
  int insertTab(int index, QWidget* widget, const QString& label);
  CloseTabIncludingDocResult closeTabIncludingDoc(core::Document* doc);
  QWidget* activeView() { return m_activeView; }
  bool tabDragging() { return m_tabDragging; }
  int indexOfPath(const QString& path);
  bool insertTabInformation(const int index);
  bool createWithSavedTabs();
  int open(const QString& path);

 public slots:
  bool closeActiveTab();
  bool closeAllTabs();
  bool closeOtherTabs();
  void addNew();
  QWidget* widget(int index) const;

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

  void setActiveView(QWidget* activeView);
  void removeTabAndWidget(int index);
  bool closeTab(QWidget* w);
  void focusTabContent(int index);
  void updateTabTextBasedOn(bool changed);

  void changeActiveView(int index);
  void changeTabText(const QString& path);
  void setTheme(const core::Theme* theme);
  void changeTabStyle(core::Theme* theme);
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint& newWindowPos, bool isFloating);
  QList<QWidget*> widgets() const;
};

Q_DECLARE_METATYPE(TabView*)
