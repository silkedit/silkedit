﻿#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>
#include <QSettings>

#include "core/macros.h"
#include "core/set_unique_ptr.h"

class TextEdit;
class TabBar;
namespace core {
class Document;
}

class TabView : public QTabWidget {
  Q_OBJECT
  DISABLE_COPY(TabView)

 public:
  enum CloseTabIncludingDocResult { UserCanceled, AllTabsRemoved, Finished };

  static const QString SETTINGS_PREFIX;

  explicit TabView(QWidget* parent = nullptr);
  ~TabView();
  DEFAULT_MOVE(TabView)

  int addTab(QWidget* page, const QString& label);
  int insertTab(int index, QWidget* widget, const QString& label);
  CloseTabIncludingDocResult closeTabIncludingDoc(core::Document* doc);
  QWidget* activeView() { return m_activeView; }
  bool tabDragging() { return m_tabDragging; }
  int indexOfPath(const QString& path);
  int open(const QString& path);
  bool closeAllTabs();
  void saveState(QSettings &settings);
  bool canSave();
  void loadState(QSettings &settings);
  QString tabTextWithoutModificationState(int index) const;

public slots:
  bool closeTab(int index);
  bool closeTab(QWidget* widget);
  void addNewTab();
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
  void focusTabContent(int index);
  void updateTabTextBasedOn(bool changed);
  void changeActiveView(int index);
  void detachTabStarted(int index, const QPoint&);
  void detachTabEntered(const QPoint& enterPoint);
  void detachTabFinished(const QPoint& newWindowPos, bool isFloating);
  QList<QWidget*> widgets() const;
};

Q_DECLARE_METATYPE(TabView*)
