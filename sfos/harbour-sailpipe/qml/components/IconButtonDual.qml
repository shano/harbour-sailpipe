import QtQuick 2.0
import Sailfish.Silica 1.0

IconButton {
    id: dualButton
    width: Theme.iconSizeMedium
    height: Theme.iconSizeMedium
    icon.sourceSize.width: width
    icon.sourceSize.height: height
    icon.fillMode: Image.PreserveAspectFit
    icon.source: ""

    property int delayInterval: 1000
    signal longClick()
    signal shortClick()

    onPressed: {
        longClickTimeout.alongun = false
        longClickTimeout.restart()
    }

    onClicked: {
        longClickTimeout.stop()
        if (!longClickTimeout.alongun) {
            shortClick()
        }
    }

    Timer {
        property bool alongun: false

        id: longClickTimeout
        repeat: false
        triggeredOnStart: false
        interval: delayInterval

        onTriggered: {
            alongun = true
            longClick()
        }
    }
}
