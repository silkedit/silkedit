#pragma once

#include "macros.h"
#include "Singleton.h"

class TextEditView;
class STabWidget;

class API : public Singleton<API> {
  DISABLE_COPY_AND_MOVE(API)

 public:
  ~API() = default;

  void init(STabWidget* tabWidget);
  TextEditView* activeEditView();

 private:
  friend class Singleton<API>;
  API() = default;

  STabWidget* m_tabWidget;
};
