#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>

#include "macros.h"
#include "set_unique_ptr.h"

class TextEditView;

class STabWidget : public QTabWidget {
  DISABLE_COPY(STabWidget)

 public:
  explicit STabWidget(QWidget* parent = nullptr);
  ~STabWidget();
  DEFAULT_MOVE(STabWidget)

  int addTab(QWidget* page, const QString& label);
  int open(const QString& path);
  void addNew();

 protected:
  void tabInserted(int index) override;

 private:
  std::unordered_set<set_unique_ptr<QWidget>> m_widgets;
};
