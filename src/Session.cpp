#include "Session.h"

void Session::setTheme(Theme *theme)
{
  m_theme = theme;
  emit themeChanged(theme);
}

Session::Session(): m_theme(nullptr)
{

}
