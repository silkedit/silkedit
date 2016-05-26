
#include "stringconv.h"

namespace string_util {

/*! Convert a QString to an std::wstring */
std::wstring qToStdWString(const QString& str) {
#ifdef _MSC_VER
  return std::wstring((const wchar_t*)str.utf16());
#else
  return str.toStdWString();
#endif
}

/*! Convert an std::wstring to a QString */
QString stdWToQString(const std::wstring& str) {
#ifdef _MSC_VER
  return QString::fromUtf16((const ushort*)str.c_str());
#else
  return QString::fromStdWString(str);
#endif
}

}  // namespace
