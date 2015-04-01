#pragma once

#include <qnamespace.h>

namespace Silk {
#ifndef Q_OS_MAC
constexpr Qt::KeyboardModifier ShiftModifier = Qt::ShiftModifier;
constexpr Qt::KeyboardModifier ControlModifier = Qt::ControlModifier;
constexpr Qt::KeyboardModifier AltModifier = Qt::AltModifier;
constexpr Qt::KeyboardModifier MetaModifier = Qt::MetaModifier;
#else
/*
Note: On Mac OS X, the ControlModifier value corresponds to the Command keys on the Macintosh
keyboard, and the MetaModifier value corresponds to the Control keys. The KeypadModifier value
will also be set when an arrow key is pressed as the arrow keys are considered part of the keypad.
Note: On Windows Keyboards, Qt::MetaModifier and Qt::Key_Meta are mapped to the Windows key.

http://qt-project.org/doc/qt-5.3/qt.html#KeyboardModifier-enum
*/

constexpr Qt::KeyboardModifier ShiftModifier = Qt::ShiftModifier;
constexpr Qt::KeyboardModifier ControlModifier = Qt::MetaModifier;
constexpr Qt::KeyboardModifier AltModifier = Qt::AltModifier;
constexpr Qt::KeyboardModifier MetaModifier = Qt::ControlModifier;
#endif

#ifndef Q_OS_MAC
enum Key {
  Key_Alt = Qt::Key_Alt,
  Key_Control = Qt::Key_Control,
  Key_Meta = Qt::Key_Meta,
  Key_Shift = Qt::Key_Shift
};
#else
enum Key {
  Key_Alt = Qt::Key_Alt,
  Key_Control = Qt::Key_Meta,
  Key_Meta = Qt::Key_Control,
  Key_Shift = Qt::Key_Shift
};
#endif
}
