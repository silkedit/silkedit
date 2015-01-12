#include <QObject>
#include <QTimer>

#include "DocumentService.h"

class Dummy : public QObject {
  Q_OBJECT
 public:
  Dummy() { QTimer::singleShot(500, this, SLOT(openFile())); }

 public slots:
  void openFile() {
    DocumentService::open("/Users/shinichi/Code/silkedit/test/testdata/test.cpp");
  }
};
