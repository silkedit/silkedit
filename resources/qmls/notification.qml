import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 600
    height: 35

    Rectangle {
        SystemPalette {
            id: palette
            colorGroup: SystemPalette.Active
        }
        id: rootRect
        color: palette.window
        border.color: "#e3e3e3"
        border.width: 1
        anchors.fill: parent

        Text {
            x: 10
            text: qsTr("SilkEdit will be updated after restart.")
            anchors.verticalCenter: parent.verticalCenter
        }

        RowLayout {
            anchors.right: rootRect.right
            anchors.verticalCenter: parent.verticalCenter

            Button {
                objectName: "updateButton"
                text: qsTr("Update Now")
                anchors.verticalCenter: parent.verticalCenter
            }
            Button {
                objectName: "laterButton"
                text: qsTr("Later")
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
