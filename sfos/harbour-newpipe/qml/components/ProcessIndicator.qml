import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property bool loading
    property int count
    property alias text: placeholder.text
    property alias hintText: placeholder.hintText
    anchors.horizontalCenter: parent.horizontalCenter

    BusyIndicator {
        visible: loading && (count === 0)
        running: visible
        size: BusyIndicatorSize.Large
        anchors.horizontalCenter: parent.horizontalCenter
        y: Math.round(Screen.height/3 - height/2)
    }

    ViewPlaceholder {
        id: placeholder
        enabled: !loading && (count === 0)
    }
}
