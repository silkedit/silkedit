#include <QtTest/QtTest>
#include <QMenuBar>

#include "util/YamlUtil.h"

namespace widgets {

class YamlUtilTest : public QObject {
  Q_OBJECT

 private slots:
  void parseMenuNode() {
    auto pkgName = "pkg";
    auto pkgPath = "path";
    QMenuBar menuBar;
    auto fileMenu = new QMenu();
    fileMenu->setObjectName("file");
    menuBar.addMenu(fileMenu);
    auto openRecentMenu = new QMenu("Open Recent");
    openRecentMenu->setObjectName("open_recent");
    fileMenu->addMenu(openRecentMenu);

    auto ymlContent = R"(
menu:
- title: File
  id: file
  menu:
  - title: 'New File'
    id: new_file
    command: new_file
  - title: 'Open'
    id: open
    command: open
    if: on_mac
  - title: 'Open File'
    id: open_file
    command: open_file
    if: on_windows
  - title: 'Open Folder'
    id: open_folder
    command: open_folder
    if: on_windows
    before: open_recent
  - title: 'Save'
    id: save
)";
    YAML::Node rootNode = YAML::Load(ymlContent);
    YAML::Node menuNode = rootNode["menu"];

    // When
    YamlUtil::parseMenuNode(pkgName, pkgPath, &menuBar, menuNode);

    // Then
    auto actions = fileMenu->actions();
    QCOMPARE(actions.size(), 6);

    QCOMPARE(actions[0]->text(), QStringLiteral("New File"));
    QCOMPARE(actions[1]->text(), QStringLiteral("Open"));
    QCOMPARE(actions[2]->text(), QStringLiteral("Open File"));
    QCOMPARE(actions[3]->text(), QStringLiteral("Open Folder"));
    QCOMPARE(actions[4]->text(), QStringLiteral("Open Recent"));
    QCOMPARE(actions[5]->text(), QStringLiteral("Save"));
  }
};

}  // namespace widgets

QTEST_MAIN(widgets::YamlUtilTest)
#include "YamlUtilTest.moc"
