#include "UniqueObject.h"

uint core::UniqueObject::s_count = 0;
QHash<int, QObject*> core::UniqueObject::s_objects;
QMutex core::UniqueObject::s_mutex;
