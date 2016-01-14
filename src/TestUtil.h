#include <QObject>
#include <QTimer>

#include "DocumentManager.h"
#include "API.h"
#include "Window.h"

class TestUtil : public QObject {
  Q_OBJECT
 public:
  TestUtil() { QTimer::singleShot(500, this, SLOT(openFile())); }

 public slots:
  void openFile() {
    DocumentManager::singleton().open("/Users/shinichi/Code/silkedit/test/testdata/test.cpp");
    DocumentManager::singleton().open("/Users/shinichi/Code/silkedit/test/testdata/test.txt");
    //    API::activeWindow()->openFindAndReplacePanel();
  }
};
