#include "HttpSendDump.h"
#include "constants.h"

#include <qdebug.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qhttpmultipart.h>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/qnetworkreply.h>

namespace CrashReport {

QString HttpSendDump::sendDump() const {
  QString ret = "";
  QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  QHttpPart pPart;
  pPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"prod\""));
#ifdef BUILD_EDGE
  pPart.setBody("SilkEdit_Edge");
#else
  pPart.setBody("SilkEdit");
#endif
  multiPart->append(pPart);

  QHttpPart vPart;
  vPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"ver\""));
  vPart.setBody(version.toUtf8());
  multiPart->append(vPart);

  QHttpPart cPart;
  cPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                  QVariant("form-data; name=\"comments\""));
  cPart.setBody(comment.toUtf8());
  multiPart->append(cPart);

  QFileInfo fileInfo(fileName);
  QHttpPart imagePart;
  imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
  imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"upload_file_minidump\"; filename=\"" +
                               fileInfo.fileName() + "\""));

  QFile* file = new QFile(fileName);
  if (!file->exists()) {
    ret = QString("Upload Error. File does not exist: ") + fileName;
    qDebug() << ret;
    return ret;
  }
  file->open(QIODevice::ReadOnly);
  imagePart.setBodyDevice(file);
  file->setParent(multiPart);  // we cannot delete the file now, so delete it with the multiPart
  multiPart->append(imagePart);

  QUrl serviceUrl(BUGSPLAT_URL);
  QNetworkRequest request(serviceUrl);
  QNetworkAccessManager manager;
  auto res = manager.post(request, multiPart);
  multiPart->setParent(res);

  QEventLoop loop;
  QObject::connect(res, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  QByteArray raw_data;
  if (res->error() == QNetworkReply::NoError) {
    raw_data = res->readAll();
    ret = QTextCodec::codecForHtml(raw_data)->toUnicode(raw_data);
  } else {
    ret = res->errorString();
  }

  delete res;

  qDebug() << ret;
  return ret;
}

}  // namespace
