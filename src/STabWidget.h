#pragma once

#include <memory>
#include <unordered_set>
#include <QTabWidget>

#include "macros.h"

class STabWidget : public QTabWidget {
  DISABLE_COPY(STabWidget)

 public:
  explicit STabWidget(QWidget* parent = nullptr);
  ~STabWidget() = default;
  DEFAULT_MOVE(STabWidget)

  int addTab(std::unique_ptr<QWidget> widget, const QString& label);

 protected:
  void tabRemoved(int index) override;

 private:
  std::unordered_set<std::unique_ptr<QWidget>> m_widgets;
};
