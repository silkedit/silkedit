#pragma once

#include <string>

#include "macros.h"

class MenuBar;

class MenuService {
  DISABLE_COPY_AND_MOVE(MenuService)

 public:
  static void init();
  static void loadMenu(const std::string& ymlPath);

 private:
  MenuService() = delete;
  ~MenuService() = delete;

  static MenuBar* m_menuBar;
};
