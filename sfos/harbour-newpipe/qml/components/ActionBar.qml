import QtQuick 2.0
import Sailfish.Silica 1.0

Row {
    id: root

    property int buttonsize: Theme.iconSizeSmallPlus

    height: buttonsize
    spacing: 2 * Theme.paddingLarge
    layoutDirection: Qt.RightToLeft

    signal fullscreenPressed
    signal downloadPressed
    signal sharePressed

    IconButton {
        width: parent.buttonsize
        height: parent.buttonsize
        icon.sourceSize.width: width
        icon.sourceSize.height: height
        icon.source: Qt.resolvedUrl("image://newpipe/icon-splus-scale?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

        onClicked: fullscreenPressed()
    }

    IconButton {
        width: parent.buttonsize
        height: parent.buttonsize
        icon.sourceSize.width: width
        icon.sourceSize.height: height
        icon.source: Qt.resolvedUrl("image://newpipe/icon-splus-cloud-download?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

        onClicked: downloadPressed()
    }

    IconButton {
        width: parent.buttonsize
        height: parent.buttonsize
        icon.sourceSize.width: width
        icon.sourceSize.height: height
        icon.source: Qt.resolvedUrl("image://newpipe/icon-splus-share?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

        onClicked: sharePressed()
    }
}
