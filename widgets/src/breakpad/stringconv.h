#pragma once

#include <qstring.h>
#include <string>

namespace string_util {
std::wstring qToStdWString(const QString&);
QString stdWToQString(const std::wstring&);
}
