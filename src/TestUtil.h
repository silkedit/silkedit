#include <QObject>
#include <QTimer>

#include "DocumentService.h"
#include "API.h"
#include "MainWindow.h"

class TestUtil : public QObject {
  Q_OBJECT
 public:
  TestUtil() { QTimer::singleShot(500, this, SLOT(openFile())); }

 public slots:
  void openFile() {
    DocumentService::open("/Users/shinichi/Code/silkedit/test/testdata/test.cpp");
    DocumentService::open("/Users/shinichi/Code/silkedit/test/testdata/test.txt");
    //    API::activeWindow()->openFindAndReplacePanel();
  }
};
