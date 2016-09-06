#include <QtTest/QtTest>

#include "BOM.h"

namespace core {

class BOMTest : public QObject {
  Q_OBJECT
 private slots:
  void guess() {
    // BOM 'a' '1'
    unsigned char bomon[] = {0xef, 0xbb, 0xbf, 0x61, 0x31};
    QByteArray bytArrBOMOn = QByteArray::fromRawData(reinterpret_cast<char*>(bomon), 5);
    QCOMPARE(BOM::guessBOM(bytArrBOMOn), BOM::getBOM(BOM::BOMSwitch::On));

    //'a' '1'
    unsigned char bomoff[] = {0x61, 0x31};
    QByteArray bytArrBOMOff = QByteArray::fromRawData(reinterpret_cast<char*>(bomoff), 2);
    QCOMPARE(BOM::guessBOM(bytArrBOMOff), BOM::getBOM(BOM::BOMSwitch::Off));

    unsigned char bomUTF16BE[] = {0xfe, 0xff, 0x61, 0x31};
    QByteArray bytArrUTF16BE = QByteArray::fromRawData(reinterpret_cast<char*>(bomUTF16BE), 4);
    QCOMPARE(BOM::guessBOM(bytArrUTF16BE), BOM::getBOM(BOM::BOMSwitch::On));

    unsigned char bomUTF16LE[] = {0xff, 0xfe, 0x61, 0x31};
    QByteArray bytArrUTF16LE = QByteArray::fromRawData(reinterpret_cast<char*>(bomUTF16LE), 4);
    QCOMPARE(BOM::guessBOM(bytArrUTF16LE), BOM::getBOM(BOM::BOMSwitch::On));
  }
};

}  // namespace core

QTEST_MAIN(core::BOMTest)
#include "BOMTest.moc"
